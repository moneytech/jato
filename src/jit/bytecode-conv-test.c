/*
 * Copyright (C) 2005  Pekka Enberg
 */

#include <statement.h>
#include <constant.h>
#include <byteorder.h>
#include <operand-stack.h>

#include <CuTest.h>
#include <stdlib.h>

static void assert_const_operand(CuTest * ct,
				 enum constant_type expected_const_type,
				 long long expected_value,
				 struct operand *operand)
{
	CuAssertIntEquals(ct, OPERAND_CONSTANT, operand->type);
	CuAssertIntEquals(ct, expected_const_type, operand->constant.type);
	CuAssertIntEquals(ct, expected_value, operand->constant.value);
}

static void assert_fconst_operand(CuTest * ct,
				  enum constant_type expected_const_type,
				  double expected_value,
				  struct operand *operand)
{
	CuAssertIntEquals(ct, OPERAND_CONSTANT, operand->type);
	CuAssertIntEquals(ct, expected_const_type, operand->constant.type);
	CuAssertDblEquals(ct, expected_value, operand->constant.fvalue, 0.01f);
}

static void assert_temporary_operand(CuTest * ct, unsigned long expected,
				     struct operand *operand)
{
	CuAssertIntEquals(ct, OPERAND_TEMPORARY, operand->type);
	CuAssertIntEquals(ct, expected, operand->temporary);
}

static void assert_arrayref_operand(CuTest * ct,
				    unsigned long expected_arrayref,
				    unsigned long expected_index,
				    struct operand *operand)
{
	CuAssertIntEquals(ct, OPERAND_ARRAYREF, operand->type);
	CuAssertIntEquals(ct, expected_arrayref, operand->arrayref);
	CuAssertIntEquals(ct, expected_index, operand->array_index);
}

static void __assert_const_stmt(CuTest * ct, struct classblock *cb,
				enum statement_type
				expected_stmt_type,
				enum constant_type
				expected_const_type,
				long long expected_value,
				char *actual, size_t count)
{
	struct operand_stack stack = OPERAND_STACK_INIT;
	struct statement *stmt =
	    convert_bytecode_to_stmts(cb, actual, count, &stack);
	CuAssertIntEquals(ct, expected_stmt_type, stmt->s_type);
	assert_const_operand(ct, expected_const_type, expected_value,
			     &stmt->s_left);

	CuAssertIntEquals(ct, stack_pop(&stack), stmt->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));

	free(stmt);
}

static void assert_const_stmt(CuTest * ct,
			      enum constant_type expected_const_type,
			      long long expected_value, char actual)
{
	unsigned char code[] = { actual };
	__assert_const_stmt(ct, NULL, STMT_ASSIGN,
			    expected_const_type, expected_value,
			    code, sizeof(code));
}

static void assert_fconst_stmt(CuTest * ct,
			       enum constant_type expected_const_type,
			       double expected_value, char actual)
{
	unsigned char code[] = { actual };
	struct operand_stack stack = OPERAND_STACK_INIT;
	struct statement *stmt =
	    convert_bytecode_to_stmts(NULL, code, sizeof(code), &stack);
	CuAssertIntEquals(ct, STMT_ASSIGN, stmt->s_type);
	assert_fconst_operand(ct, expected_const_type, expected_value,
			      &stmt->s_left);

	CuAssertIntEquals(ct, stack_pop(&stack), stmt->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));

	free(stmt);
}

void test_convert_nop(CuTest * ct)
{
	unsigned char code[] = { OPC_NOP };
	struct operand_stack stack = OPERAND_STACK_INIT;
	struct statement *stmt =
	    convert_bytecode_to_stmts(NULL, code, sizeof(code), &stack);
	CuAssertIntEquals(ct, STMT_NOP, stmt->s_type);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));
	free(stmt);
}

void test_convert_aconst_null(CuTest * ct)
{
	assert_const_stmt(ct, CONST_REFERENCE, 0, OPC_ACONST_NULL);
}

void test_convert_iconst(CuTest * ct)
{
	assert_const_stmt(ct, CONST_INT, -1, OPC_ICONST_M1);
	assert_const_stmt(ct, CONST_INT, 0, OPC_ICONST_0);
	assert_const_stmt(ct, CONST_INT, 1, OPC_ICONST_1);
	assert_const_stmt(ct, CONST_INT, 2, OPC_ICONST_2);
	assert_const_stmt(ct, CONST_INT, 3, OPC_ICONST_3);
	assert_const_stmt(ct, CONST_INT, 4, OPC_ICONST_4);
	assert_const_stmt(ct, CONST_INT, 5, OPC_ICONST_5);
}

void test_convert_lconst(CuTest * ct)
{
	assert_const_stmt(ct, CONST_LONG, 0, OPC_LCONST_0);
	assert_const_stmt(ct, CONST_LONG, 1, OPC_LCONST_1);
}

void test_convert_fconst(CuTest * ct)
{
	assert_fconst_stmt(ct, CONST_FLOAT, 0, OPC_FCONST_0);
	assert_fconst_stmt(ct, CONST_FLOAT, 1, OPC_FCONST_1);
	assert_fconst_stmt(ct, CONST_FLOAT, 2, OPC_FCONST_2);
}

void test_convert_dconst(CuTest * ct)
{
	assert_fconst_stmt(ct, CONST_DOUBLE, 0, OPC_DCONST_0);
	assert_fconst_stmt(ct, CONST_DOUBLE, 1, OPC_DCONST_1);
}

static void assert_bipush_stmt(CuTest * ct, char expected_value, char actual)
{
	unsigned char code[] = { actual, expected_value };
	__assert_const_stmt(ct, NULL, STMT_ASSIGN, CONST_INT,
			    expected_value, code, sizeof(code));
}

void test_convert_bipush(CuTest * ct)
{
	assert_bipush_stmt(ct, 0x00, OPC_BIPUSH);
	assert_bipush_stmt(ct, 0x01, OPC_BIPUSH);
	assert_bipush_stmt(ct, 0xFF, OPC_BIPUSH);
}

static void assert_sipush_stmt(CuTest * ct,
			       long long expected_value,
			       char first, char second, char actual)
{
	unsigned char code[] = { actual, first, second };
	__assert_const_stmt(ct, NULL, STMT_ASSIGN, CONST_INT,
			    expected_value, code, sizeof(code));
}

#define MIN_SHORT (-32768)
#define MAX_SHORT 32767

void test_convert_sipush(CuTest * ct)
{
	assert_sipush_stmt(ct, 0, 0x00, 0x00, OPC_SIPUSH);
	assert_sipush_stmt(ct, 1, 0x00, 0x01, OPC_SIPUSH);
	assert_sipush_stmt(ct, MIN_SHORT, 0x80, 0x00, OPC_SIPUSH);
	assert_sipush_stmt(ct, MAX_SHORT, 0x7F, 0xFF, OPC_SIPUSH);
}

static struct statement *convert_bytecode_with_cp(ConstantPoolEntry * cp_infos,
						  size_t nr_cp_infos,
						  u1 * cp_types,
						  unsigned char opcode,
						  unsigned char index1,
						  unsigned char index2,
						  struct operand_stack *stack)
{
	struct classblock cb = {
		.constant_pool_count = sizeof(cp_infos),
		.constant_pool.info = cp_infos,
		.constant_pool.type = cp_types
	};
	unsigned char code[] = { opcode, index1, index2 };
	return convert_bytecode_to_stmts(&cb, code, sizeof(code), stack);
}

static void assert_ldc_stmt(CuTest * ct,
			    enum constant_type expected_const_type,
			    long expected_value, u1 cp_type)
{
	ConstantPoolEntry cp_infos[] = { cpu_to_be64(expected_value) };
	u1 cp_types[] = { cp_type };
	struct operand_stack stack = OPERAND_STACK_INIT;

	struct statement *stmt =
	    convert_bytecode_with_cp(cp_infos, sizeof(cp_infos),
				     cp_types, OPC_LDC, 0x00,
				     0x00,
				     &stack);
	CuAssertIntEquals(ct, STMT_ASSIGN, stmt->s_type);
	assert_const_operand(ct, expected_const_type, expected_value,
			     &stmt->s_left);
	CuAssertIntEquals(ct, stack_pop(&stack), stmt->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));
	free(stmt);
}

static void assert_ldc_stmt_float(CuTest * ct, float expected_value)
{
	u4 value = *(u4 *) & expected_value;
	ConstantPoolEntry cp_infos[] = { cpu_to_be64(value) };
	u1 cp_types[] = { CONSTANT_Float };
	struct operand_stack stack = OPERAND_STACK_INIT;

	struct statement *stmt =
	    convert_bytecode_with_cp(cp_infos, sizeof(cp_infos),
				     cp_types, OPC_LDC, 0x00,
				     0x00,
				     &stack);
	CuAssertIntEquals(ct, STMT_ASSIGN, stmt->s_type);
	assert_fconst_operand(ct, CONST_FLOAT, expected_value, &stmt->s_left);
	CuAssertIntEquals(ct, stack_pop(&stack), stmt->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));
	free(stmt);
}

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX - 1)

void test_convert_ldc(CuTest * ct)
{
	assert_ldc_stmt(ct, CONST_INT, 0, CONSTANT_Integer);
	assert_ldc_stmt(ct, CONST_INT, 1, CONSTANT_Integer);
	assert_ldc_stmt(ct, CONST_INT, INT_MIN, CONSTANT_Integer);
	assert_ldc_stmt(ct, CONST_INT, INT_MAX, CONSTANT_Integer);
	assert_ldc_stmt_float(ct, 0.01f);
	assert_ldc_stmt_float(ct, 1.0f);
	assert_ldc_stmt_float(ct, -1.0f);
	assert_ldc_stmt(ct, CONST_REFERENCE, 0xDEADBEEF, CONSTANT_String);
}

static void assert_ldcw_stmt(CuTest * ct,
			     enum constant_type expected_const_type,
			     long long expected_value, u1 cp_type,
			     unsigned char opcode)
{
	ConstantPoolEntry cp_infos[257];
	cp_infos[256] = cpu_to_be64(expected_value);
	u1 cp_types[257];
	cp_types[256] = cp_type;
	struct operand_stack stack = OPERAND_STACK_INIT;

	struct statement *stmt =
	    convert_bytecode_with_cp(cp_infos, sizeof(cp_infos),
				     cp_types, opcode, 0x01,
				     0x00,
				     &stack);
	CuAssertIntEquals(ct, STMT_ASSIGN, stmt->s_type);
	assert_const_operand(ct, expected_const_type, expected_value,
			     &stmt->s_left);
	CuAssertIntEquals(ct, stack_pop(&stack), stmt->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));
	free(stmt);
}

static void __assert_ldcw_stmt_double(CuTest * ct,
				      enum constant_type
				      expected_constant_type,
				      double expected_value,
				      u1 cp_type, u8 value,
				      unsigned long opcode)
{
	ConstantPoolEntry cp_infos[257];
	cp_infos[256] = cpu_to_be64(value);
	u1 cp_types[257];
	cp_types[256] = cp_type;
	struct operand_stack stack = OPERAND_STACK_INIT;

	struct statement *stmt =
	    convert_bytecode_with_cp(cp_infos, sizeof(cp_infos),
				     cp_types, opcode, 0x01,
				     0x00,
				     &stack);
	CuAssertIntEquals(ct, STMT_ASSIGN, stmt->s_type);
	assert_fconst_operand(ct, expected_constant_type, expected_value,
			      &stmt->s_left);
	CuAssertIntEquals(ct, stack_pop(&stack), stmt->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));
	free(stmt);
}

static void assert_ldcw_stmt_float(CuTest * ct,
				   enum constant_type expected_constant_type,
				   float expected_value, u1 cp_type,
				   unsigned long opcode)
{
	u4 value = *(u4 *) & expected_value;
	__assert_ldcw_stmt_double(ct, expected_constant_type,
				  expected_value, cp_type, value, opcode);
}

static void assert_ldcw_stmt_double(CuTest * ct,
				    enum constant_type
				    expected_constant_type,
				    double expected_value,
				    u1 cp_type, unsigned long opcode)
{
	u8 value = *(u8 *) & expected_value;
	__assert_ldcw_stmt_double(ct, expected_constant_type,
				  expected_value, cp_type, value, opcode);
}

void test_convert_ldc_w(CuTest * ct)
{
	assert_ldcw_stmt(ct, CONST_INT, 0, CONSTANT_Integer, OPC_LDC_W);
	assert_ldcw_stmt(ct, CONST_INT, 1, CONSTANT_Integer, OPC_LDC_W);
	assert_ldcw_stmt(ct, CONST_INT, INT_MIN, CONSTANT_Integer, OPC_LDC_W);
	assert_ldcw_stmt(ct, CONST_INT, INT_MAX, CONSTANT_Integer, OPC_LDC_W);
	assert_ldcw_stmt_float(ct, CONST_FLOAT, 0.01f, CONSTANT_Float,
			       OPC_LDC_W);
	assert_ldcw_stmt_float(ct, CONST_FLOAT, 1.0f, CONSTANT_Float,
			       OPC_LDC_W);
	assert_ldcw_stmt_float(ct, CONST_FLOAT, -1.0f, CONSTANT_Float,
			       OPC_LDC_W);
	assert_ldcw_stmt(ct, CONST_REFERENCE, 0xDEADBEEF, CONSTANT_String,
			 OPC_LDC_W);
}

#define LONG_MAX ((long long) 2<<63)
#define LONG_MIN (-LONG_MAX - 1)

void test_convert_ldc2_w(CuTest * ct)
{
	assert_ldcw_stmt(ct, CONST_LONG, 0, CONSTANT_Long, OPC_LDC2_W);
	assert_ldcw_stmt(ct, CONST_LONG, 1, CONSTANT_Long, OPC_LDC2_W);
	assert_ldcw_stmt(ct, CONST_LONG, LONG_MIN, CONSTANT_Long, OPC_LDC2_W);
	assert_ldcw_stmt(ct, CONST_LONG, LONG_MAX, CONSTANT_Long, OPC_LDC2_W);
	assert_ldcw_stmt_double(ct, CONST_DOUBLE, 0.01f, CONSTANT_Double,
				OPC_LDC2_W);
	assert_ldcw_stmt_double(ct, CONST_DOUBLE, 1.0f, CONSTANT_Double,
				OPC_LDC2_W);
	assert_ldcw_stmt_double(ct, CONST_DOUBLE, -1.0f, CONSTANT_Double,
				OPC_LDC2_W);
}

static void assert_xload_stmt(CuTest * ct, unsigned char opc,
			      enum jvm_type expected_jvm_type,
			      unsigned char expected_index)
{
	unsigned char code[] = { opc, expected_index };
	struct operand_stack stack = OPERAND_STACK_INIT;
	struct statement *stmt =
	    convert_bytecode_to_stmts(NULL, code, sizeof(code), &stack);
	CuAssertIntEquals(ct, STMT_ASSIGN, stmt->s_type);
	CuAssertIntEquals(ct, OPERAND_LOCAL_VAR, stmt->s_left.type);
	CuAssertIntEquals(ct, expected_index, stmt->s_left.local_var.index);
	CuAssertIntEquals(ct, expected_jvm_type,
			  stmt->s_left.local_var.type);
	CuAssertIntEquals(ct, stack_pop(&stack), stmt->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));
	free(stmt);
}

void test_convert_iload(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_ILOAD, J_INT, 0x00);
	assert_xload_stmt(ct, OPC_ILOAD, J_INT, 0x01);
	assert_xload_stmt(ct, OPC_ILOAD, J_INT, 0xFF);
}

void test_convert_lload(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_LLOAD, J_LONG, 0x00);
	assert_xload_stmt(ct, OPC_LLOAD, J_LONG, 0x01);
	assert_xload_stmt(ct, OPC_LLOAD, J_LONG, 0xFF);
}

void test_convert_fload(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_FLOAD, J_FLOAT, 0x00);
	assert_xload_stmt(ct, OPC_FLOAD, J_FLOAT, 0x01);
	assert_xload_stmt(ct, OPC_FLOAD, J_FLOAT, 0xFF);
}

void test_convert_dload(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_DLOAD, J_DOUBLE, 0x00);
	assert_xload_stmt(ct, OPC_DLOAD, J_DOUBLE, 0x01);
	assert_xload_stmt(ct, OPC_DLOAD, J_DOUBLE, 0xFF);
}

void test_convert_aload(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_ALOAD, J_REFERENCE, 0x00);
	assert_xload_stmt(ct, OPC_ALOAD, J_REFERENCE, 0x01);
	assert_xload_stmt(ct, OPC_ALOAD, J_REFERENCE, 0xFF);
}

void test_convert_iload_x(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_ILOAD_0, J_INT, 0x00);
	assert_xload_stmt(ct, OPC_ILOAD_1, J_INT, 0x01);
	assert_xload_stmt(ct, OPC_ILOAD_2, J_INT, 0x02);
	assert_xload_stmt(ct, OPC_ILOAD_3, J_INT, 0x03);
}

void test_convert_lload_x(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_LLOAD_0, J_LONG, 0x00);
	assert_xload_stmt(ct, OPC_LLOAD_1, J_LONG, 0x01);
	assert_xload_stmt(ct, OPC_LLOAD_2, J_LONG, 0x02);
	assert_xload_stmt(ct, OPC_LLOAD_3, J_LONG, 0x03);
}

void test_convert_fload_x(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_FLOAD_0, J_FLOAT, 0x00);
	assert_xload_stmt(ct, OPC_FLOAD_1, J_FLOAT, 0x01);
	assert_xload_stmt(ct, OPC_FLOAD_2, J_FLOAT, 0x02);
	assert_xload_stmt(ct, OPC_FLOAD_3, J_FLOAT, 0x03);
}

void test_convert_dload_x(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_DLOAD_0, J_DOUBLE, 0x00);
	assert_xload_stmt(ct, OPC_DLOAD_1, J_DOUBLE, 0x01);
	assert_xload_stmt(ct, OPC_DLOAD_2, J_DOUBLE, 0x02);
	assert_xload_stmt(ct, OPC_DLOAD_3, J_DOUBLE, 0x03);
}

void test_convert_aload_x(CuTest * ct)
{
	assert_xload_stmt(ct, OPC_ALOAD_0, J_REFERENCE, 0x00);
	assert_xload_stmt(ct, OPC_ALOAD_1, J_REFERENCE, 0x01);
	assert_xload_stmt(ct, OPC_ALOAD_2, J_REFERENCE, 0x02);
	assert_xload_stmt(ct, OPC_ALOAD_3, J_REFERENCE, 0x03);
}

static void assert_null_check_stmt(CuTest *ct, unsigned long expected_ref, struct statement *actual)
{
	CuAssertIntEquals(ct, STMT_NULL_CHECK, actual->s_type);
	assert_temporary_operand(ct, expected_ref, &actual->s_left);
}

static void assert_arraycheck_stmt(CuTest *ct, unsigned long expected_arrayref, unsigned long expected_index, struct statement *actual)
{
	CuAssertIntEquals(ct, STMT_ARRAY_CHECK, actual->s_type);
	assert_temporary_operand(ct, expected_arrayref, &actual->s_left);
	assert_temporary_operand(ct, expected_index, &actual->s_right);
}

static void assert_xaload_stmts(CuTest * ct, unsigned char opc,
				unsigned long arrayref, unsigned long index)
{
	unsigned char code[] = { opc };
	struct operand_stack stack = OPERAND_STACK_INIT;
	stack_push(&stack, arrayref);
	stack_push(&stack, index);

	struct statement *stmt =
	    convert_bytecode_to_stmts(NULL, code, sizeof(code), &stack);

	struct statement *nullcheck = stmt;
	struct statement *arraycheck = stmt->s_next;
	struct statement *assign = arraycheck->s_next;

	assert_null_check_stmt(ct, arrayref, nullcheck);
	assert_arraycheck_stmt(ct, arrayref, index, arraycheck);
	CuAssertIntEquals(ct, STMT_ASSIGN, assign->s_type);
	assert_arrayref_operand(ct, arrayref, index, &assign->s_left);

	CuAssertIntEquals(ct, stack_pop(&stack), assign->s_target);
	CuAssertIntEquals(ct, true, stack_is_empty(&stack));
}

void test_convert_iaload(CuTest * ct)
{
	assert_xaload_stmts(ct, OPC_IALOAD, 0, 1);
	assert_xaload_stmts(ct, OPC_IALOAD, 1, 2);
}

void test_convert_laload(CuTest * ct)
{
	assert_xaload_stmts(ct, OPC_LALOAD, 0, 1);
	assert_xaload_stmts(ct, OPC_LALOAD, 1, 2);
}

void test_convert_faload(CuTest * ct)
{
	assert_xaload_stmts(ct, OPC_FALOAD, 0, 1);
	assert_xaload_stmts(ct, OPC_FALOAD, 1, 2);
}

void test_convert_daload(CuTest * ct)
{
	assert_xaload_stmts(ct, OPC_DALOAD, 0, 1);
	assert_xaload_stmts(ct, OPC_DALOAD, 1, 2);
}

void test_convert_aaload(CuTest * ct)
{
	assert_xaload_stmts(ct, OPC_AALOAD, 0, 1);
	assert_xaload_stmts(ct, OPC_AALOAD, 1, 2);
}
