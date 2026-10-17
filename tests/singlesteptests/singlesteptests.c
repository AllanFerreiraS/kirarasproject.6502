#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../include/processor_6502.h"
#include "../../include/memory_16.h"
#include "../../thirdparty/cJson/cJSON.h"

#define TRUE			1
#define FALSE			0
#define TOTAL_TESTS		10000
#define ERROR_MESSAGE_SIZE	256

typedef struct {
	uint16_t passed;
	uint16_t failed;
} passed_failed_counter;

// Store the name of the test and what error occurred.
typedef struct {
	char * test_name;
	char * error;
} feedback;

// Others Function
static char * allocate_message_error(void);
static void clean_memory(memory_16 * memory); // MUDAR DEPOIS
static char * get_json_content(const char * filename);
static void 	set_reg_error(const char * reg_name, uint64_t got, uint64_t expected, char **error_message);

// JSON ACCESS
static uint8_t 	get_json_cycles(cJSON **test, uint64_t * cycles);
static uint8_t 	get_json_test_name(cJSON **test, char ** test_name);
static uint8_t 	store_json_processor_content(processor_6502 * processor, cJSON **test, const char * field);
static uint8_t 	store_json_memory_content(memory_16 * memory, cJSON **test, const char * field);

// Comparisons
static uint8_t compare_processor(processor_6502 * p_cpu, processor_6502 * p_expected, char **error_message);
static uint8_t compare_memory(memory_16 * m_mem, memory_16 * m_expected, char **error_message);
static uint8_t compare_cycles(processor_6502 * processor, uint64_t cycles, char **error_message);

// Init process
static void free_feedbacks(feedback *f, uint16_t size);
static void print_results(passed_failed_counter counter, feedback *feedbacks);
static uint8_t init_test(processor_6502 * p_cpu, processor_6502 * p_expected, memory_16 * m_mem, memory_16 * m_expected, cJSON **root);



static char * allocate_message_error(void)
{
	return (char *) malloc(ERROR_MESSAGE_SIZE);
}

static void clean_memory(memory_16 * memory)
{
	for (uint32_t i = 0; i < MEMORY_16_SIZE; i++) {
		memory_write(memory, i, 0);
	}
}

static char * get_json_content(const char * filename)
{
	FILE *file = fopen(filename, "r");

	if (file == NULL)
        	return NULL;
	
	// Get File Size
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char * buffer = (char *) malloc(file_size + 1);
	fread(buffer, 1, file_size, file);
	buffer[file_size] = '\0';
	fclose(file);

	return buffer;
}

static uint8_t get_json_cycles(cJSON **test, uint64_t * cycles)
{
	cJSON * content = cJSON_GetObjectItemCaseSensitive(*test, "cycles");

	if (content == NULL) {
		printf("Error when tried to get json cycles");
		return FALSE;
	}
	
	*cycles = (uint64_t) cJSON_GetArraySize(content);
	return TRUE;
}

static uint8_t get_json_test_name(cJSON **test, char ** test_name)
{
	cJSON *content = cJSON_GetObjectItemCaseSensitive(*test, "name");

	if (content == NULL) {
		printf("Error when tried to get json test name.\n");
		return FALSE;
	}

	*test_name = content->valuestring;
	return TRUE;
}

static uint8_t store_json_processor_content(processor_6502 * processor, cJSON **test, const char * field)
{
	cJSON *content 	= cJSON_GetObjectItemCaseSensitive(*test, field);

	if (content == NULL) {
		printf("Error when tried to store json processor content.\n");
		return FALSE;
	}
	
	cJSON *pc = cJSON_GetObjectItemCaseSensitive(content, "pc");
	if (!cJSON_IsNumber(pc)) {
		printf("Error: Invalid initial PC\n");
		return FALSE;
	}

	cJSON *s = cJSON_GetObjectItemCaseSensitive(content, "s");
	if (!cJSON_IsNumber(s)) {
		printf("Error: Invalid initial S\n");
		return FALSE;
	}

	cJSON *a = cJSON_GetObjectItemCaseSensitive(content, "a");
	if (!cJSON_IsNumber(a)) {
		printf("Error: Invalid initial A\n");
		return FALSE;
	}

	cJSON *x = cJSON_GetObjectItemCaseSensitive(content, "x");
	if (!cJSON_IsNumber(x)) {
		printf("Error: Invalid initial X\n");
		return FALSE;
	}

	cJSON *y = cJSON_GetObjectItemCaseSensitive(content, "y");
	if (!cJSON_IsNumber(y)) {
		printf("Error: Invalid initial Y\n");
		return FALSE;
	}

	cJSON *p = cJSON_GetObjectItemCaseSensitive(content, "p");
	if (!cJSON_IsNumber(p)) {
		printf("Error: Invalid initial P\n");
		return FALSE;
	}

	processor->reg_pc	= (uint16_t) 	pc->valueint;
	processor->reg_sp 	= (uint8_t)  	s->valueint;
	processor->reg_a 	= (uint8_t) 	a->valueint;
	processor->reg_x 	= (uint8_t) 	x->valueint;
	processor->reg_y 	= (uint8_t) 	y->valueint;
	processor->reg_p 	= (uint8_t) 	p->valueint;
	processor->cycles	= 0;

	return TRUE;
}

static uint8_t store_json_memory_content(memory_16 * memory, cJSON **test, const char * field)
{
	cJSON *content 	= cJSON_GetObjectItemCaseSensitive(*test, field);

	clean_memory(memory);
	
	if (content == NULL) {
		printf("Error when tried to store json memory content.\n");
		return FALSE;
	}

	cJSON *ram = cJSON_GetObjectItemCaseSensitive(content, "ram");
	cJSON *entry;

	cJSON_ArrayForEach(entry, ram) {
		cJSON * addr 	= cJSON_GetArrayItem(entry, 0);
		cJSON * value	= cJSON_GetArrayItem(entry, 1);

		if (!cJSON_IsNumber(addr) || !cJSON_IsNumber(value)) {
			printf("Error: Invalid memory values");
			return FALSE;
		}

		memory_write(memory, (uint16_t)addr->valueint, (uint8_t)value->valueint);
	}
	
	return TRUE;
}

static void set_reg_error(const char * reg_name, uint64_t got, uint64_t expected, char **error_message)
{
	*error_message = allocate_message_error();
	if (*error_message == NULL)
		return;

	snprintf(*error_message, ERROR_MESSAGE_SIZE, "PROCESSOR REG_%s: got %lu expected %lu.\n", reg_name, got, expected);
	return;	
}

static uint8_t compare_processor(processor_6502 * p_cpu, processor_6502 * p_expected, char **error_message)
{
	if (! (p_cpu->reg_pc == p_expected->reg_pc) ) {
		set_reg_error("PC", p_cpu->reg_pc, p_expected->reg_pc, error_message);
		return FALSE;
	}

	if (! (p_cpu->reg_sp == p_expected->reg_sp) ) {
		set_reg_error("SP", p_cpu->reg_sp, p_expected->reg_sp, error_message);
		return FALSE;
	}

	if (! (p_cpu->reg_a == p_expected->reg_a) ) {
		set_reg_error("A", p_cpu->reg_a, p_expected->reg_a, error_message);
		return FALSE;
	}

	if (! (p_cpu->reg_x == p_expected->reg_x) ) {
		set_reg_error("X", p_cpu->reg_x, p_expected->reg_x, error_message);
		return FALSE;
	}

	if (! (p_cpu->reg_y == p_expected->reg_y) ) {
		set_reg_error("Y", p_cpu->reg_y, p_expected->reg_y, error_message);
		return FALSE;
	}

	if (! (p_cpu->reg_p == p_expected->reg_p) ) {
		set_reg_error("P", p_cpu->reg_p, p_expected->reg_p, error_message);
		return FALSE;
	}

	return TRUE;
}

static uint8_t compare_memory(memory_16 * m_mem, memory_16 * m_expected, char **error_message)
{
	uint8_t data_old, data_new;
	data_old = data_new = 0;

	for (uint32_t i = 0; i < MEMORY_16_SIZE; i++) {
		data_old = memory_read(m_mem, i);
		data_new = memory_read(m_expected, i);
		if (data_old != data_new) {
			*error_message = allocate_message_error();
			snprintf(*error_message, ERROR_MESSAGE_SIZE, "MEMORY: got memory[%d]=%d expected memory[%d]=%d\n", i, data_old, i, data_new);
			return FALSE;
		}
	}
	return TRUE;
}

static uint8_t compare_cycles(processor_6502 * processor, uint64_t cycles, char **error_message)
{
	if (processor->cycles != cycles) {
		set_reg_error("CYCLES", processor->cycles, cycles, error_message);
		return FALSE;
	}
	return TRUE;
}

static void free_feedbacks(feedback *f, uint16_t size)
{
	for (uint16_t i = 0; i < size; i++)
		if (f[i].error != NULL)
			free(f[i].error);
}

static void print_results(passed_failed_counter counter, feedback *feedbacks)
{
	char flag;

	printf("\nRESULT: (%u/%u)\n", counter.passed, TOTAL_TESTS);
	printf("Errors: ");

	if (counter.failed == 0) {
		printf("NONE.\n");
	}
	else {
		printf("%d.\n", counter.failed);
		for (uint16_t i = 0; i < counter.failed; i++) {
			printf("Test Name: %s -> ", feedbacks[i].test_name);
			if (feedbacks[i].error == NULL)
				printf("It's not possible to show the error message.\n");
			else
				printf("%s\n", feedbacks[i].error);
			printf("Press any for next (q for exit): ");
			scanf("%c", &flag);
			if (flag == 'q')
				break;
		}
	}
}

static uint8_t init_test(processor_6502 * p_cpu, processor_6502 * p_expected, memory_16 * m_mem, memory_16 * m_expected, cJSON **root)
{
	cJSON *test;
	passed_failed_counter counter;
	feedback feedbacks[TOTAL_TESTS];

	char * test_name = NULL;
	char * error_message = NULL;
	uint64_t cycles = 0;
	
	counter.passed = 0;
	counter.failed = 0;

	printf("Testing...\n");
	cJSON_ArrayForEach(test, *root) {
		
		if (! store_json_processor_content(p_cpu, &test, "initial") ) {
			free_feedbacks(feedbacks, counter.failed);
			return FALSE;
		}

		if (! store_json_processor_content(p_expected, &test, "final") ) {
			free_feedbacks(feedbacks, counter.failed);
			return FALSE;
		}

		if (! store_json_memory_content(m_mem, &test, "initial") ) {
			free_feedbacks(feedbacks, counter.failed);
			return FALSE;
		}

		if (! store_json_memory_content(m_expected, &test, "final") ) {
			free_feedbacks(feedbacks, counter.failed);
			return FALSE;
		}

		if (! get_json_cycles(&test, &cycles) ) {
			free_feedbacks(feedbacks, counter.failed);
			return FALSE;
		}

		if (! get_json_test_name(&test, &test_name) ) {
			free_feedbacks(feedbacks, counter.failed);
			return FALSE;
		}

		execute_opcode(p_cpu, m_mem);

		if (! compare_processor(p_cpu, p_expected, &error_message) ) {
			feedbacks[counter.failed].test_name = test_name;
			feedbacks[counter.failed].error = error_message;
			counter.failed++;
			goto goto_end;
		}

		if (! compare_memory(m_mem, m_expected, &error_message) ) {
			feedbacks[counter.failed].test_name = test_name;
			feedbacks[counter.failed].error = error_message;
			counter.failed++;
			goto goto_end;
		}

		if (! compare_cycles(p_cpu, cycles, &error_message) ) {
			feedbacks[counter.failed].test_name = test_name;
			feedbacks[counter.failed].error = error_message;
			counter.failed++;
			goto goto_end;
		}

		counter.passed++;

goto_end:
		// Preparing for next process
		cycles = 0;
	}
	
	print_results(counter, feedbacks);
	free_feedbacks(feedbacks, counter.failed);
	return TRUE;
}

int main (int argc, char *argv[])
{
	if (argc <= 1) {
		printf("Specify the opcode in hexadecimal (without \'0x\').\n");
		return 1;
	}

	processor_6502 p_cpu, p_expected;
	memory_16 m_mem, m_expected;

	char * filename = allocate_message_error(); // Preguiça de mudar o nome da função...
	snprintf(filename, ERROR_MESSAGE_SIZE, "./v1/%s.json", argv[1]);

	char * content = get_json_content(filename);
	if (content == NULL) {
		printf("Error opening file: %s\n", filename);
		free(filename);
		exit(EXIT_FAILURE);
	}

	cJSON *root = cJSON_Parse(content);	
	if (root == NULL) {
		printf("Error: JSON_ROOT is NULL\n");
		cJSON_Delete(root);
		free(content);
		free(filename);
		exit(EXIT_FAILURE);
	}

	if (! init_test(&p_cpu, &p_expected, &m_mem, &m_expected, &root) )
		printf("Error: Stoping test...\n");
	
	cJSON_Delete(root);
	free(content);
	free(filename);
	return 0;
}
