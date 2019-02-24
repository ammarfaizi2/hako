
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <esteh/error.hpp>
#include <esteh/vm/opcode.hpp>
#include <esteh/vm/estehvm.hpp>
#include <esteh/vm/code_parser.hpp>

#include "escape_char.hpp"

#define ESTEH_DIR_OPCACHE "__teacache__"

code_parser::code_parser() {
}

void code_parser::finish() {

}

int code_parser::is_ok() {
	return 1;
}

char *code_parser::get_error() {
	return nullptr;
}

size_t code_parser::get_error_length() {
	return 0;
}

void code_parser::set_file(char *filename) {
	this->filename = filename;
	this->file_fd = open(this->filename, O_RDONLY);
	struct stat st;
	if (fstat(this->file_fd, &st)) {
		esteh_error("Could not open file \"%s\"", this->filename);
		exit(1);
	}
	this->filesize = st.st_size;
	this->map = (char *)mmap(NULL, this->filesize, PROT_READ|PROT_WRITE, MAP_PRIVATE, this->file_fd, 0);
}

void code_parser::init_opcache_dir() {
	struct stat st = {0};
		
	if (stat(ESTEH_DIR_OPCACHE, &st) == -1) {
		#ifdef ESTEH_DEBUG
			printf("mkdir %s\n", ESTEH_DIR_OPCACHE);
		#endif
	    mkdir(ESTEH_DIR_OPCACHE, 0700);
	}

}

void code_parser::build_opcode() {

	#define $rb this->map[i]
	this->init_opcache_dir();

	esteh_opcode **opcodes = (esteh_opcode **)malloc(sizeof(esteh_opcode *));
	uint32_t opcode_count = this->parse_file(&opcodes);
	int skip = 0;	
	
	#ifdef ESTEH_DEBUG
		if (this->error_parse != nullptr) {
			printf("Error Parse: %s\n\n", this->error_parse);
		}
	#endif

	// Run opcode.
	for (uint32_t i = 0; i < opcode_count; ++i) {

		if (skip) {
			skip = 0;
			continue;
		}

		switch (opcodes[i]->code) {
			case TD_PRINT:

				switch (opcodes[i+1]->code) {
					case TE_INT:
						fprintf(stdout, "%d", *((int *)opcodes[i + 1]->content));
					break;
					case TE_STRING:
						fprintf(stdout, "%s", (char *)opcodes[i+1]->content);
					break;
				}

				free(opcodes[i+1]->content);
				opcodes[i+1]->content = nullptr;
				free(opcodes[i+1]);
				opcodes[i+1] = nullptr;
				skip = 1;
			break;
		}
		free(opcodes[i]);
		opcodes[i] = nullptr;
	}

	free(opcodes);
	opcodes = nullptr;
}

uint32_t code_parser::parse_file(esteh_opcode ***opcodes) {

	#define $opc (*opcodes)

	#define __er0 "Syntax Error: Unknown token \"%s\" in \"%s\" on line %d\n"
	#define __er1 "Syntax Error: Unterminated operation in \"%s\" on line %d\n"

	#define UNKNOWN_TOKEN \
		this->error_parse = (char *)malloc( \
			sizeof(char) * (sizeof(__er0) + strlen(token) + (floor(log10(line)) + 1) + strlen(this->filename)) \
		); \
		sprintf(this->error_parse, __er0, token, this->filename, line);

	#define UNTERMINATED_OP \
		this->error_parse = (char *)malloc( \
			sizeof(char) * ((floor(log10(line)) + 1) + strlen(this->filename)) \
		); \
		sprintf(this->error_parse, __er1, this->filename, line);

	#define CLEAND goto cleand

	// Parser conditions.
	uint32_t in_dquo = 0;
	uint32_t in_te = 0;
	uint32_t in_int = 0;
	// uint32_t end_op = 0;
	uint32_t dquo_escaped = 0;
	uint32_t line = 1;

	char *token = (char *)malloc(sizeof(char));
	size_t token_size = 0;

	uint32_t opcode_count;

	for (size_t i = 0; i < this->filesize; ++i) {
		if ($rb == '"') {
			if (in_dquo) {
				// End of a string.

				// Got an opcode.
				token[token_size] = '\0';
				$opc = (esteh_opcode **)realloc($opc, sizeof(esteh_opcode *) * (opcode_count + 1));
				$opc[opcode_count] = (esteh_opcode *)malloc(sizeof(esteh_opcode));
				$opc[opcode_count]->line = line;
				$opc[opcode_count]->code = TE_STRING;
				$opc[opcode_count]->content = (char *)malloc(sizeof(char) * (token_size + 1));
				memcpy($opc[opcode_count]->content, token, sizeof(char) * (token_size + 1));

				opcode_count++;
				in_dquo = 0;
				token_size = 0;
			} else {
				// Start of a string.
				in_dquo = 1;
			}
			CLEAND;

		} else if (in_dquo) {

			if ((!dquo_escaped) && $rb == '\\') {
				dquo_escaped = 1;
				CLEAND;
			}

			if (dquo_escaped) {
				$rb = this->escape_char($rb);
				dquo_escaped = 0;
			}

			token = (char *)realloc(token, token_size + 2);
			token[token_size] = $rb;
			token_size++;
		}

		if (
			(!in_dquo) &&
			(($rb >= 65 && $rb <= 90) ||
			($rb >= 97 && $rb <= 122) ||
			($rb == 95))
		) {
			if (!in_te) {
				in_te = 1;
			}
			token = (char *)realloc(token, token_size + 2);
			token[token_size] = $rb;
			token_size++;
			CLEAND;
		} else if (in_te) {

			if ($rb >= 48 && $rb <= 57) {
				token = (char *)realloc(token, token_size + 2);
				token[token_size] = $rb;
				token_size++;
				continue;
			}

			// Got an opcode.
			token[token_size] = '\0';
			$opc = (esteh_opcode **)realloc($opc, sizeof(esteh_opcode *) * (opcode_count + 1));
			$opc[opcode_count] = (esteh_opcode *)malloc(sizeof(esteh_opcode));
			$opc[opcode_count]->line = line;
			$opc[opcode_count]->code = this->token_d(token);
			if (($opc[opcode_count]->code = this->token_d(token)) == T_UNKNOWN) {
				UNKNOWN_TOKEN
				return 0;
			}
			$opc[opcode_count]->content = nullptr;
			opcode_count++;
			in_te = 0;
			token_size = 0;
		}

		if (
			(!in_dquo) && (!in_te) &&
			($rb >= 48 && $rb <= 57)
		) {
			if (!in_int) {
				in_int = 1;
			}
			token = (char *)realloc(token, token_size + 2);
			token[token_size] = $rb;
			token_size++;
			CLEAND;
		} else if (in_int) {
			// Got an opcode.
			token[token_size] = '\0';
			$opc = (esteh_opcode **)realloc($opc, sizeof(esteh_opcode *) * (opcode_count + 1));
			$opc[opcode_count] = (esteh_opcode *)malloc(sizeof(esteh_opcode));
			$opc[opcode_count]->line = line;
			$opc[opcode_count]->code = TE_INT;
			$opc[opcode_count]->content = malloc(sizeof(int));
			*((int *)$opc[opcode_count]->content) = atoi(token);
			opcode_count++;
			in_int = 0;
			token_size = 0;
		}

		if (!($rb == 9 || $rb == 32 || $rb == 10)) {

		}


		cleand:		
		if ($rb == 10) {
			line++;
		}
	}

	free(token);
	token = nullptr;

	munmap(this->map, this->filesize);
	this->map = nullptr;
	close(this->file_fd);
	return opcode_count;
}
