#!/bin/bash
# paranoid_stress.sh - Intense stability lockdown for 64-bit SVR4 shell
# Author: Antigravity (Mario's collaborator)

ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)
SH="$ROOT_DIR/sh"

# Android/Bionic aborts on ulimit -v violations
if [ -d /system ] || [ -n "$TERMUX_VERSION" ]; then
    ULIMIT_CMD="ulimit -t 60"
else
    ULIMIT_CMD="ulimit -v 1024000 -t 60" # 1GB RAM, 60s CPU
fi

echo "===================================================="
echo "   PARANOID MODE STRESS TEST: SVR4 BOURNE SHELL      "
echo "===================================================="

run_stress() {
    local name="$1"
    local script="$2"
    echo -n "[Stress] $name... "
    
    # Run the script through the target shell
    start_time=$(date +%s%N)
    # Use single quotes for SH -c but here we are using heredoc
    # The bash -c wrapper needs to handle the heredoc safely
    output=$(bash -c "$ULIMIT_CMD && $SH 2>&1 <<'STRESS_EOF'
$script
STRESS_EOF")
    exit_code=$?
    end_time=$(date +%s%N)
    
    # Check if end_time is empty or invalid (some shells)
    if [ -z "$end_time" ] || [ -z "$start_time" ]; then
        duration="unknown"
    else
        duration=$(( (end_time - start_time) / 1000000 ))
    fi
    
    if [ $exit_code -eq 0 ]; then
        echo "OK (${duration}ms)"
    else
        echo "FAILED (Code: $exit_code, ${duration}ms)"
        echo "---- OUTPUT ----"
        echo "$output"
        echo "----------------"
        return 1
    fi
}

# 1. Heredoc Torture (Expansion & Redirection)
run_stress "Heredoc Deep Expansion" '
X=outer
Y=inner
CAT=/usr/bin/cat
[ -f /system/bin/cat ] && CAT=/system/bin/cat
$CAT <<EOF1
$X
`$CAT <<EOF2
$Y
EOF2
`
EOF1
'

# 2. Pipe Chain Torture (30 stages)
run_stress "Pipe Chain (30 stages)" '
CAT=/usr/bin/cat
GREP=/usr/bin/grep
[ -f /system/bin/cat ] && CAT=/system/bin/cat
[ -f /system/bin/grep ] && GREP=/system/bin/grep
echo "start" | $CAT | $CAT | $CAT | $CAT | $CAT | \
$CAT | $CAT | $CAT | $CAT | $CAT | \
$CAT | $CAT | $CAT | $CAT | $CAT | \
$CAT | $CAT | $CAT | $CAT | $CAT | \
$CAT | $CAT | $CAT | $CAT | $CAT | \
$CAT | $CAT | $CAT | $CAT | $GREP "start"
'

# 3. Subshell Recursion Torture
run_stress "Deep Subshell Nesting" '
( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( echo deep ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) )
'

# 4. Trap & Signal Churn
run_stress "Trap/Signal Churn" '
SLEEP=/usr/bin/sleep
[ -f /system/bin/sleep ] && SLEEP=/system/bin/sleep
trap "echo trapped" 15
for i in 1 2 3 4 5; do
    $SLEEP 0.1 &
    trap "echo reset" 15
done
wait
echo done
'

# 5. Environment & Export Torture
run_stress "Environment Churn" '
ENV=/usr/bin/env
GREP=/usr/bin/grep
[ -f /system/bin/env ] && ENV=/system/bin/env
[ -f /system/bin/grep ] && GREP=/system/bin/grep
for i in 1 2 3 4 5 6 7 8 9 10; do
    eval "VAR$i=val$i"
    eval "export VAR$i"
done
$ENV | $GREP VAR10
'

# 6. Large Argument List Torture (Stack/Heap interaction)
run_stress "Large Argument Churn" '
/system/bin/echo 1 2 3 4 5 6 7 8 9 0 > /dev/null 2>&1 || /usr/bin/echo 1 2 3 4 5 6 7 8 9 0 > /dev/null
echo 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 \
     1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 \
     1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 \
     > /dev/null
'

# 7. Command Substitution Loop
run_stress "Cmd Subst Loop" '
EXPR=/usr/bin/expr
ECHO=echo
[ -f /system/bin/expr ] && EXPR=/system/bin/expr
i=0
while [ $i -lt 10 ]; do
    x=`$ECHO $i`
    i=`$EXPR $i + 1`
done
echo $i
'

echo "===================================================="
echo "STRESS TEST COMPLETE"
echo "===================================================="
