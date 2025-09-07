#!/bin/bash

# 编译和运行测试的脚本

echo "=== ASan User States 测试编译和运行指南 ==="
echo

# 检查编译器
LLVM_CLANG="/home/leo/project/llvm-project/build/bin/clang++"
if [ ! -f "$LLVM_CLANG" ]; then
    echo "错误: LLVM clang++ 编译器未找到"
    echo "请确保LLVM项目已正确编译"
    exit 1
fi

echo "✓ 找到LLVM编译器: $LLVM_CLANG"
echo

# 设置编译选项
INCLUDES="-I/home/leo/project/llvm-project/compiler-rt/include -I/home/leo/project/llvm-project/compiler-rt/include/sanitizer"
CFLAGS="-fsanitize=address -g"

echo "编译选项:"
echo "  编译器: $LLVM_CLANG"
echo "  标志: $CFLAGS"
echo "  包含路径: $INCLUDES"
echo

# 编译所有测试
echo "正在编译测试程序..."
echo

# 定义要编译的测试
tests=(
    "basic_functionality_test.cpp:test_basic"
    "api_comprehensive_test.cpp:test_api"
    "performance_test.cpp:test_perf"
    "simple_demo.cpp:simple_demo"
    "violation_detection_test.cpp:test_violation"
    "violation_types_test.cpp:test_destroyed"
)

for test_config in "${tests[@]}"; do
    source_file="${test_config%%:*}"
    output_file="${test_config##*:}"
    
    echo "编译 $source_file -> $output_file"
    $LLVM_CLANG $CFLAGS $INCLUDES $source_file -o $output_file
    
    if [ $? -eq 0 ]; then
        echo "✓ 编译成功"
    else
        echo "✗ 编译失败"
        exit 1
    fi
    echo
done

echo "所有测试编译完成！"
echo
echo "现在可以运行测试:"
echo "  ./test_basic     - 基础功能测试"
echo "  ./test_api       - API综合测试"
echo "  ./test_perf      - 性能测试"
echo "  ./simple_demo    - 简化演示"
echo "  ./test_violation - 违规检测测试"
echo "  ./test_destroyed - 已销毁内存测试"
echo
echo "运行完整测试套件:"
echo "  ./run_tests.sh"