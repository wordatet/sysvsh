#!/bin/bash
# paranoid_stress.sh - Intense stability lockdown for 64-bit SVR4 shell
# Author: Antigravity (Mario's collaborator)

SH="../sh"
if [ ! -f "$SH" ]; then (cd .. && make sh) || exit 1; fi
ULIMIT_CMD="ulimit -v 1024000 -t 60" # 1GB RAM, 60s CPU

echo "===================================================="
echo "   PARANOID MODE STRESS TEST: SVR4 BOURNE SHELL      "
echo "===================================================="

run_stress() {
    local name="$1"
    local script="$2"
    echo -n "[Stress] $name... "
    
    # Run the script through the target shell
    start_time=$(date +%s%N)
    output=$(bash -c "$ULIMIT_CMD && $SH 2>&1 <<'STRESS_EOF'
$script
STRESS_EOF")
    exit_code=$?
    end_time=$(date +%s%N)
    
    duration=$(( (end_time - start_time) / 1000000 ))
    
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
echo "start" | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | \
/usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | \
/usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | \
/usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | \
/usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | \
/usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/cat | /usr/bin/grep "start"
'

# 3. Subshell Recursion Torture
run_stress "Deep Subshell Nesting" '
( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( ( echo deep ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) ) )
'

# 4. Trap & Signal Churn
run_stress "Trap/Signal Churn" '
trap "echo trapped" 15
for i in 1 2 3 4 5; do
    /usr/bin/sleep 0.1 &
    trap "echo reset" 15
done
wait
echo done
'

# 5. Environment & Export Torture
run_stress "Environment Churn" '
for i in 1 2 3 4 5 6 7 8 9 10; do
    eval "VAR$i=val$i"
    eval "export VAR$i"
done
/usr/bin/env | /usr/bin/grep VAR10
'

# 6. Large Argument List Torture (Stack/Heap interaction)
run_stress "Large Argument Churn" '
/usr/bin/echo 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 \
              1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 \
              1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 \
              > /dev/null
'

# 7. Command Substitution Loop
run_stress "Cmd Subst Loop" '
i=0
while [ $i -lt 10 ]; do
    x=`/usr/bin/echo $i`
    i=`/usr/bin/expr $i + 1`
done
echo $i
'

echo "===================================================="
echo "STRESS TEST COMPLETE"
echo "===================================================="
