#!/bin/bash
# verify_port.sh - Automated verification for the 64-bit SVR4 shell port

SH="../sh"
ULIMIT_CMD="ulimit -v 256000 -t 5"

fail_count=0
pass_count=0

run_test() {
    local desc="$1"
    local cmd="$2"
    local expected="$3"
    
    echo -n "Test: $desc... "
    # Run with ulimit to prevent system hangs
    output=$(bash -c "$ULIMIT_CMD && $SH -c '$cmd'" 2>&1)
    exit_code=$?
    
    if [ $exit_code -eq 0 ] && echo "$output" | grep -q "$expected"; then
        echo "PASS"
        ((pass_count++))
    else
        echo "FAIL (Exit: $exit_code)"
        echo "  Expected match: $expected"
        echo "  Output: $output"
        ((fail_count++))
    fi
}

echo "Starting automated verification..."
(cd .. && make sh) || exit 1

run_test "Simple echo" "echo hello" "hello"
run_test "Pipe support" "echo hello | /usr/bin/wc -c" "6"
run_test "Variable substitution" "x=foo; echo \$x" "foo"
run_test "For loop" "for i in a b; do echo \$i; done" "a"
run_test "Semicolon" "echo hi; echo lo" "hi"
run_test "Heredoc" "/usr/bin/cat <<EOF
test
EOF" "test"

echo "--------------------------------"
echo "Results: $pass_count PASS, $fail_count FAIL"

if [ $fail_count -eq 0 ]; then
    echo "Port appears stable!"
    exit 0
else
    echo "Stability issues persist."
    exit 1
fi
