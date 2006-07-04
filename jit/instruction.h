#ifndef __JIT_INSTRUCTION_H
#define __JIT_INSTRUCTION_H

#include <vm/list.h>

struct statement;

enum reg {
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	REG_EBP,
	REG_ESP,
};

struct operand {
	union {
		enum reg reg;
		struct {
			enum reg base_reg;
			unsigned long disp;	/* displacement */
		};
		unsigned long imm;
		unsigned long rel;
	};
};

enum insn_opcode {
	OPC_ADD,
	OPC_CALL,
	OPC_CMP,
	OPC_JE,
	OPC_MOV,
	OPC_PUSH,
};

enum operand_type {
	OPERAND_RELATIVE,	/* Call target */
	OPERAND_REGISTER,	/* Register */
	OPERAND_IMMEDIATE,	/* Immediate value */
	OPERAND_MEMBASE,	/* Memory operand consisting of base offset
				   in register with immediate value
				   displacement.  */
	OPERAND_BRANCH,		/* Branch target */
};

#define OPERAND_TYPE_SHIFT 8UL

#define SRC_OPERAND_TYPE_SHIFT OPERAND_TYPE_SHIFT
#define DEST_OPERAND_TYPE_SHIFT 16UL

/*	Instruction that operates on one operand.  */
#define DEFINE_INSN_TYPE_1(opc, operand_type) (((operand_type) << OPERAND_TYPE_SHIFT) | (opc))

/*	Instruction that operates on two operands. */
#define DEFINE_INSN_TYPE_2(opc, src_operand_type, dest_operand_type) \
	(((src_operand_type) << SRC_OPERAND_TYPE_SHIFT) |((dest_operand_type) << DEST_OPERAND_TYPE_SHIFT) | (opc))

/*
 *	Instruction type identifies the opcode, number of operands, and
 * 	operand types.
 */
enum insn_type {
	INSN_ADD_DISP_REG	= DEFINE_INSN_TYPE_2(OPC_ADD,  OPERAND_MEMBASE,   OPERAND_REGISTER),
	INSN_ADD_IMM_REG	= DEFINE_INSN_TYPE_2(OPC_ADD,  OPERAND_IMMEDIATE, OPERAND_REGISTER),
	INSN_CALL_REL		= DEFINE_INSN_TYPE_1(OPC_CALL, OPERAND_RELATIVE),
	INSN_CMP_DISP_REG	= DEFINE_INSN_TYPE_2(OPC_CMP,  OPERAND_MEMBASE,   OPERAND_REGISTER),
	INSN_JE_BRANCH		= DEFINE_INSN_TYPE_1(OPC_JE,   OPERAND_BRANCH),
	INSN_MOV_DISP_REG	= DEFINE_INSN_TYPE_2(OPC_MOV,  OPERAND_MEMBASE,   OPERAND_REGISTER),
	INSN_MOV_IMM_REG	= DEFINE_INSN_TYPE_2(OPC_MOV,  OPERAND_IMMEDIATE, OPERAND_REGISTER),
	INSN_MOV_IMM_DISP	= DEFINE_INSN_TYPE_2(OPC_MOV,  OPERAND_IMMEDIATE, OPERAND_MEMBASE),
	INSN_PUSH_IMM		= DEFINE_INSN_TYPE_1(OPC_PUSH, OPERAND_IMMEDIATE),
	INSN_PUSH_REG		= DEFINE_INSN_TYPE_1(OPC_PUSH, OPERAND_REGISTER),
};

struct insn {
	enum insn_type type;
	union {
		struct {
			struct operand src;
			struct operand dest;
		};
		struct operand operand;
	};
	struct statement *branch_target;
	struct list_head insn_list_node;
	struct list_head branch_list_node;
	unsigned long offset;
};


static inline struct insn *insn_entry(struct list_head *head)
{
	return list_entry(head, struct insn, insn_list_node);
}

struct insn *membase_reg_insn(enum insn_opcode, enum reg, unsigned long, enum reg);
struct insn *reg_insn(enum insn_opcode, enum reg);
struct insn *imm_reg_insn(enum insn_opcode, unsigned long, enum reg);
struct insn *imm_membase_insn(enum insn_opcode, unsigned long, enum reg, unsigned long);
struct insn *imm_insn(enum insn_opcode, unsigned long);
struct insn *rel_insn(enum insn_opcode, unsigned long);
struct insn *branch_insn(enum insn_opcode, struct statement *);

struct insn *alloc_insn(enum insn_type);
void free_insn(struct insn *);

#endif
