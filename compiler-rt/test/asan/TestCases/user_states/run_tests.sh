#!/bin/bash

# ASan User States 测试运行脚本
# 这个脚本会运行所有测试并展示结果

echo "======================================"
echo "ASan User States 完整测试套件"
echo "======================================"
echo

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试计数器
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# 运行测试函数
run_test() {
    local test_name="$1"
    local test_command="$2"
    local expect_crash="$3"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    echo -e "${YELLOW}运行测试 $TOTAL_TESTS: $test_name${NC}"
    
    if [ "$expect_crash" = "true" ]; then
        # 预期会崩溃的测试
        if timeout 3s bash -c "$test_command" >/dev/null 2>&1; then
            echo -e "${RED}✗ 失败: 程序应该崩溃但没有崩溃${NC}"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        else
            echo -e "${GREEN}✓ 通过: 程序按预期崩溃${NC}"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        fi
    else
        # 正常测试
        if $test_command >/dev/null 2>&1; then
            echo -e "${GREEN}✓ 通过${NC}"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}✗ 失败${NC}"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    fi
    echo
}

# 运行详细测试函数
run_detailed_test() {
    local test_name="$1"
    local test_command="$2"
    
    echo "--------------------------------------"
    echo "$test_name"
    echo "--------------------------------------"
    $test_command
    echo
}

echo "第一部分: 功能测试"
echo "=================="

# 运行功能测试
run_detailed_test "基础功能测试" "./test_basic"
run_detailed_test "API综合测试" "./test_api"
run_detailed_test "性能测试" "./test_perf"
run_detailed_test "简化演示" "./simple_demo"

echo "第二部分: 违规检测测试"
echo "===================="

echo "注意: 以下测试会触发ASan并终止程序，这是正常行为"
echo

run_test "写入只读内存违规检测" "./test_violation" "true"
run_test "访问已销毁内存违规检测" "./test_destroyed" "true"

echo "第三部分: 完整演示"
echo "==============="

run_detailed_test "完整功能演示" "./demo"

echo "======================================"
echo "测试结果汇总"
echo "======================================"
echo "总测试数: $TOTAL_TESTS"
echo -e "${GREEN}通过: $PASSED_TESTS${NC}"
echo -e "${RED}失败: $FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}🎉 所有测试通过！ASan User States功能正常工作！${NC}"
    exit 0
else
    echo -e "${RED}❌ 有 $FAILED_TESTS 个测试失败${NC}"
    exit 1
fi