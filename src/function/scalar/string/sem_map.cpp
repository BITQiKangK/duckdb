#include "duckdb/function/scalar/string_functions.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/common/vector_operations/unary_executor.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckdb/llm/llm_call.hpp"

namespace duckdb {

// 实现sem_map函数的操作
struct SemMapOperator {
	template <class INPUT_TYPE, class RESULT_TYPE>
	static RESULT_TYPE Operation(INPUT_TYPE input, Vector &result) {
		auto input_data = input.GetData();
		auto input_length = input.GetSize();
		
		std::string mapped_str = single_round_llm_call(std::string(input_data, input_length));
		
		auto result_str = StringVector::AddString(result, mapped_str);
		return result_str;
	}
};

// 注册sem_map函数
static void SemMapFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	UnaryExecutor::ExecuteString<string_t, string_t, SemMapOperator>(args.data[0], result, args.size());
}

// 获取函数定义
ScalarFunction SemMapFun::GetFunction() {
	return ScalarFunction("sem_map", {LogicalType::VARCHAR}, LogicalType::VARCHAR, SemMapFunction);
}

} // namespace duckdb 