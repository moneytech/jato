/*
 * Copyright (C) 2005-2006  Pekka Enberg
 *
 * This file is released under the GPL version 2. Please refer to the file
 * LICENSE for details.
 *
 * The file contains functions for converting Java bytecode load and store
 * instructions to immediate representation of the JIT compiler.
 */

#include <jit/statement.h>
#include <vm/bytecode.h>
#include <vm/byteorder.h>
#include <vm/stack.h>
#include <jit/jit-compiler.h>
#include <bytecodes.h>

#include <errno.h>

static int __convert_const(enum vm_type vm_type,
			   unsigned long long value, struct stack *expr_stack)
{
	struct expression *expr = value_expr(vm_type, value);
	if (!expr)
		return -ENOMEM;

	stack_push(expr_stack, expr);
	return 0;
}

int convert_aconst_null(struct compilation_unit *cu,
			       struct basic_block *bb, unsigned long offset)
{
	return __convert_const(J_REFERENCE, 0, cu->expr_stack);
}

int convert_iconst(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return __convert_const(J_INT, read_u8(code, offset) - OPC_ICONST_0,
			       cu->expr_stack);
}

int convert_lconst(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return __convert_const(J_LONG, read_u8(code, offset) - OPC_LCONST_0,
			       cu->expr_stack);
}

static int __convert_fconst(enum vm_type vm_type,
			    double value, struct stack *expr_stack)
{
	struct expression *expr = fvalue_expr(vm_type, value);
	if (!expr)
		return -ENOMEM;

	stack_push(expr_stack, expr);
	return 0;
}

int convert_fconst(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return __convert_fconst(J_FLOAT,
				read_u8(code, offset) - OPC_FCONST_0,
				cu->expr_stack);
}

int convert_dconst(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return __convert_fconst(J_DOUBLE,
				read_u8(code, offset) - OPC_DCONST_0,
				cu->expr_stack);
}

int convert_bipush(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return __convert_const(J_INT, read_s8(code, offset + 1),
			       cu->expr_stack);
}

int convert_sipush(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return __convert_const(J_INT,
			       read_s16(code, offset + 1),
			       cu->expr_stack);
}

static int __convert_ldc(struct constant_pool *cp,
			 unsigned long cp_idx, struct stack *expr_stack)
{
	struct expression *expr;

	u1 type = CP_TYPE(cp, cp_idx);
	switch (type) {
	case CONSTANT_Integer:
		expr = value_expr(J_INT, CP_INTEGER(cp, cp_idx));
		break;
	case CONSTANT_Float:
		expr = fvalue_expr(J_FLOAT, CP_FLOAT(cp, cp_idx));
		break;
	case CONSTANT_String:
		expr = value_expr(J_REFERENCE, CP_STRING(cp, cp_idx));
		break;
	case CONSTANT_Long:
		expr = value_expr(J_LONG, CP_LONG(cp, cp_idx));
		break;
	case CONSTANT_Double:
		expr = fvalue_expr(J_DOUBLE, CP_DOUBLE(cp, cp_idx));
		break;
	default:
		return -EINVAL;
	}

	if (!expr)
		return -ENOMEM;

	stack_push(expr_stack, expr);
	return 0;
}

int convert_ldc(struct compilation_unit *cu, struct basic_block *bb,
		       unsigned long offset)
{
	struct classblock *cb = CLASS_CB(cu->method->class);
	unsigned char *code = cu->method->jit_code;
	return __convert_ldc(&cb->constant_pool,
			     read_u8(code, offset + 1), cu->expr_stack);
}

int convert_ldc_w(struct compilation_unit *cu, struct basic_block *bb,
			 unsigned long offset)
{
	struct classblock *cb = CLASS_CB(cu->method->class);
	unsigned char *code = cu->method->jit_code;
	return __convert_ldc(&cb->constant_pool,
			     read_u16(code, offset + 1),
			     cu->expr_stack);
}

int convert_ldc2_w(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	struct classblock *cb = CLASS_CB(cu->method->class);
	unsigned char *code = cu->method->jit_code;
	return __convert_ldc(&cb->constant_pool,
			     read_u16(code, offset + 1),
			     cu->expr_stack);
}

int convert_load(struct compilation_unit *cu,
			struct basic_block *bb,
			unsigned char index, enum vm_type type)
{
	struct expression *expr;

	expr = local_expr(type, index);
	if (!expr)
		return -ENOMEM;

	stack_push(cu->expr_stack, expr);
	return 0;
}

int convert_iload(struct compilation_unit *cu, struct basic_block *bb,
			 unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset + 1), J_INT);
}

int convert_lload(struct compilation_unit *cu, struct basic_block *bb,
			 unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset + 1), J_LONG);
}

int convert_fload(struct compilation_unit *cu, struct basic_block *bb,
			 unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset + 1), J_FLOAT);
}

int convert_dload(struct compilation_unit *cu, struct basic_block *bb,
			 unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset + 1), J_DOUBLE);
}

int convert_aload(struct compilation_unit *cu, struct basic_block *bb,
			 unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset + 1), J_REFERENCE);
}

int convert_iload_n(struct compilation_unit *cu, struct basic_block *bb,
			   unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset) - OPC_ILOAD_0, J_INT);
}

int convert_lload_n(struct compilation_unit *cu, struct basic_block *bb,
			   unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset) - OPC_LLOAD_0, J_LONG);
}

int convert_fload_n(struct compilation_unit *cu, struct basic_block *bb,
			   unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset) - OPC_FLOAD_0, J_FLOAT);
}

int convert_dload_n(struct compilation_unit *cu, struct basic_block *bb,
			   unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset) - OPC_DLOAD_0, J_DOUBLE);
}

int convert_aload_n(struct compilation_unit *cu, struct basic_block *bb,
			   unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_load(cu, bb, read_u8(code, offset) - OPC_ALOAD_0,
			    J_REFERENCE);
}

static int convert_store(struct compilation_unit *cu,
			 struct basic_block *bb,
			 enum vm_type type, unsigned long index)
{
	struct expression *src_expr, *dest_expr;
	struct statement *stmt = alloc_statement(STMT_STORE);
	if (!stmt)
		goto failed;

	dest_expr = local_expr(type, index);
	src_expr = stack_pop(cu->expr_stack);

	stmt->store_dest = &dest_expr->node;
	stmt->store_src = &src_expr->node;
	bb_add_stmt(bb, stmt);
	return 0;
      failed:
	free_statement(stmt);
	return -ENOMEM;
}

int convert_istore(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_INT, read_u8(code, offset + 1));
}

int convert_lstore(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_LONG, read_u8(code, offset + 1));
}

int convert_fstore(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_FLOAT, read_u8(code, offset + 1));
}

int convert_dstore(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_DOUBLE, read_u8(code, offset + 1));
}

int convert_astore(struct compilation_unit *cu, struct basic_block *bb,
			  unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_REFERENCE, read_u8(code, offset + 1));
}

int convert_istore_n(struct compilation_unit *cu, struct basic_block *bb,
			    unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_INT, read_u8(code, offset) - OPC_ISTORE_0);
}

int convert_lstore_n(struct compilation_unit *cu, struct basic_block *bb,
			    unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_LONG, read_u8(code, offset) - OPC_LSTORE_0);
}

int convert_fstore_n(struct compilation_unit *cu, struct basic_block *bb,
			    unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_FLOAT, read_u8(code, offset) - OPC_FSTORE_0);
}

int convert_dstore_n(struct compilation_unit *cu, struct basic_block *bb,
			    unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_DOUBLE, read_u8(code, offset) - OPC_DSTORE_0);
}

int convert_astore_n(struct compilation_unit *cu, struct basic_block *bb,
			    unsigned long offset)
{
	unsigned char *code = cu->method->jit_code;
	return convert_store(cu, bb, J_REFERENCE,
			     read_u8(code, offset) - OPC_ASTORE_0);
}
