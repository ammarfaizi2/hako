
#include <cstdio>
#include <esteh/vm/opcode.hpp>
#include <esteh/vm/code_parser.hpp>

#define _OPCODES_ALLOC sizeof(esteh_opcode *) * 10;
#define _OPCODES_REALLOC \
	if ( (((*opcode_count) + 2) * sizeof(esteh_opcode *)) >= *cur_opcodes_alloc ) { \
		*cur_opcodes_alloc = (((*opcode_count) + 2) * sizeof(esteh_opcode *)) + _OPCODES_ALLOC \
		(*opcodes) = (esteh_opcode **)realloc(*opcodes, *cur_opcodes_alloc); \
	} \

void code_parser::build_opcode(
	uint32_t opcode_offset,
	uint32_t tokens_count,
	uint32_t *opcode_count,
	uint32_t *cur_opcodes_alloc,
	bool opcode_must_has_operand,
	esteh_token **tokens,
	esteh_opcode ***opcodes
) {
	
	_OPCODES_REALLOC

	switch (tokens[opcode_offset]->token) {
		case TD_PRINT:
			(*opcodes)[*opcode_count] = (esteh_opcode *)malloc(sizeof(esteh_opcode));
			(*opcodes)[*opcode_count]->code = TD_PRINT;
			(*opcodes)[*opcode_count]->lineno = tokens[opcode_offset]->lineno;
			uint32_t i = opcode_offset + 1;
			bool skip = false;

			while (i < tokens_count) {
				if (skip) {
					i++;
					skip = false;
					continue;
				}
				switch (tokens[i]->token) {

					case TE_STRING:
					case TE_INT:
						(*opcodes)[*opcode_count]->op1.static_value.value = tokens[i]->val.value;
						(*opcodes)[*opcode_count]->op1.static_value.type = tokens[i]->val.type;
						(*opcodes)[*opcode_count]->op1_type = static_value;
					break;

					case TF_ADD:
					case TF_MIN:
					case TF_MUL:
					case TF_DIV:
						skip = true;
						(*opcodes)[*opcode_count]->op1_type = opcode_1;
						(*opcodes)[*opcode_count]->op1.opcode_1 = (esteh_opcode *)malloc(sizeof(esteh_opcode));
						(*opcodes)[*opcode_count]->op1.opcode_1->code = tokens[i]->token;
						if (i < 2 && (tokens_count <= (i + 1))) {
							// Error parse.
						} else {

							if (tokens[i - 1]->token == TE_INT) {
								(*opcodes)[*opcode_count]->op1.opcode_1->op1.static_value.value = tokens[i - 1]->val.value;
								(*opcodes)[*opcode_count]->op1.opcode_1->op1.static_value.type = ESTEH_TYPE_INT;
							} else {
								// Must be variable or error parse.
							}

							if (tokens[i - 1]->token == TE_INT) {
								(*opcodes)[*opcode_count]->op1.opcode_1->op2.static_value.value = tokens[i + 1]->val.value;
								(*opcodes)[*opcode_count]->op1.opcode_1->op2.static_value.type = ESTEH_TYPE_INT;
							} else {
								// Must be variable or error parse.
							}
							(*opcodes)[*opcode_count]->op1.opcode_1->result_type = static_value;
							(*opcodes)[*opcode_count]->op1.opcode_1->result.static_value.type = ESTEH_TYPE_INT;
						}
						
					break;
				}
				i++;
			}

			(*opcode_count)++;
		break;
	}

}