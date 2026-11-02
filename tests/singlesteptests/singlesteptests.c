#include "./../../include/infrastructure.h"
#include "./../../include/hardwares/processor_6502.h"
#include "./../../include/hardwares/memory_16.h"
#include "./../../emulators/raw/raw.h"
#include "./../../thirdparty/cJson/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

extern uint16_t c_cycles[3 * 256];
extern uint16_t c_cycles_i;

// Others Function
static char	* allocate_message_error(void);
static void 	clean_memory(memory_16 * memory); // MUDAR DEPOIS
static char 	* get_json_content(const char * filename);
static void 	set_reg_error(const char * reg_name, uint64_t got, uint64_t expected, char **error_message);

// JSON ACCESS
static uint8_t 	get_json_test_name(cJSON **test, char ** test_name);
static uint8_t 	store_json_cycles(cJSON **test, uint16_t **cycles_expected, uint16_t * cycles_size);
static uint8_t 	store_json_processor_content(processor_6502 * processor, cJSON **test, const char * field);
static uint8_t 	store_json_memory_content(memory_16 * memory, cJSON **test, const char * field);

// Comparisons
static uint8_t compare_processor(processor_6502 * p_cpu, processor_6502 * p_expected, char **error_message);
static uint8_t compare_memory(memory_16 * m_mem, memory_16 * m_expected, char **error_message);
static uint8_t compare_cycles(uint16_t *cycles, uint16_t size, uint16_t *cycles_expected, uint16_t size_expected, char **error_message);


// Init process
static void 	free_feedbacks(feedback *f, uint16_t size);
static void 	print_results(passed_failed_counter counter, feedback *feedbacks);
static uint8_t 	init_test(processor_6502 * p_cpu, processor_6502 * p_expected, memory_16 * m_mem, memory_16 * m_expected, cJSON **root);

// Main
uint8_t get_json_legal_opcodes(uint8_t *arr, uint16_t *size);
uint8_t all_files_test(infra_raw * emulator, infra_raw * emulator_expected);
uint8_t single_file_test(const char * file, infra_raw * emulator, infra_raw * emulator_expected);


void u8_decimal_to_binary_string(uint8_t v, char * b)
{
	uint8_t i = 0;
	uint8_t mask = 1;
	for (; i < 8; i++) {
		b[i] = ( (v & (mask << (7-i))) > 0 ) ? '1' : '0';
	}
	b[i] = '\0';

}

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

static uint8_t store_json_cycles(cJSON **test, uint16_t **cycles_expected, uint16_t * cycles_size)
{
	cJSON 	 *content = cJSON_GetObjectItemCaseSensitive(*test, "cycles");
	cJSON 	 *entry;

	uint16_t *c;
	uint8_t  cycle_arr_size = 0;
	uint16_t i = 0;

	if (content == NULL) {
		printf("Error when tried to get json cycles");
		return FALSE;
	}

	cycle_arr_size = (uint8_t) cJSON_GetArraySize(content);
	c = (uint16_t *) malloc(3 * cycle_arr_size * sizeof(uint16_t));
		
	cJSON_ArrayForEach(entry, content) {
		cJSON * addr 		= cJSON_GetArrayItem(entry, 0);
		cJSON * value		= cJSON_GetArrayItem(entry, 1);
		cJSON * read_write	= cJSON_GetArrayItem(entry, 2);
		
		if (!cJSON_IsNumber(addr) || !cJSON_IsNumber(value) || !cJSON_IsString(read_write)) {
			printf("Error: Invalid memory values");
			free(c);
			return FALSE;
		}
		
		c[i++] = (uint16_t) addr->valueint;
		c[i++] = (uint16_t) value->valueint;
		c[i++] = (uint16_t) ((strcmp((char *)read_write->valuestring, "write") == 0) ? 1 : 0);	
	}

	*cycles_expected = c;
	*cycles_size 	 = (uint16_t) cycle_arr_size * 3;
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
	char got_bin[9] = {0};
	char expected_bin[9] = {0};

	if (*error_message == NULL)
		return;	
	if ( strcmp(reg_name,"P") == 0 ) {
		u8_decimal_to_binary_string((uint8_t) got, got_bin);
		u8_decimal_to_binary_string((uint8_t) expected, expected_bin);
		snprintf(*error_message, ERROR_MESSAGE_SIZE, "PROCESSOR REG_%s: got %lu (0b%s) expected %lu (0b%s).\n", reg_name, got, got_bin, expected, expected_bin);
		return;
	} else {
		snprintf(*error_message, ERROR_MESSAGE_SIZE, "PROCESSOR REG_%s: got %lu expected %lu.\n", reg_name, got, expected);
	}
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

static uint8_t compare_cycles(uint16_t *cycles, uint16_t size, uint16_t *cycles_expected, uint16_t size_expected, char **error_message)
{
	if (size != size_expected) {
		*error_message = allocate_message_error();
		snprintf(*error_message, ERROR_MESSAGE_SIZE, "CYCLES COUNTER: got %d expected %d.\n", size/3, size_expected/3);
		return FALSE;
	}

	for (uint8_t i = 0; i < (size_expected / 3); i++)
		for (uint8_t j = 0; j < 3; j++)
			if (cycles[j + (i * 3)] != cycles_expected[j + (i * 3)]) {
				*error_message = allocate_message_error();
				snprintf(*error_message, ERROR_MESSAGE_SIZE, "CYCLES: got [%d, %d, %d] expected [%d, %d, %d].\n", cycles[0 + (i * 3)], cycles[1 + (i * 3)], cycles[2 + (i * 3)], cycles_expected[0 + (i * 3)], cycles_expected[1 + (i * 3)], cycles_expected[2 + (i * 3)]);
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

	char 	 *test_name 	= NULL;
	char 	 *error_message = NULL;

	uint16_t *cycles_expected 	= NULL;
	uint16_t cycles_expected_size 	= 0;
	
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

		if (! store_json_cycles(&test, &cycles_expected, &cycles_expected_size) ) {
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
		
		if (! compare_cycles(c_cycles, c_cycles_i, cycles_expected, cycles_expected_size, &error_message)
 ) {
			feedbacks[counter.failed].test_name = test_name;
			feedbacks[counter.failed].error = error_message;
			counter.failed++;
			goto goto_end;
		}
		
		counter.passed++;

goto_end:
		//Preparing cycles for next process
		free(cycles_expected);
		cycles_expected_size = 0;
		c_cycles_i = 0;
	}
	
	print_results(counter, feedbacks);
	free_feedbacks(feedbacks, counter.failed);
	return TRUE;
}


uint8_t get_json_legal_opcodes(uint8_t *arr, uint16_t *size)
{

	uint8_t return_status	= TRUE;

	for (uint16_t i = 0; i < 0x100; i++)
		arr[i] = 0;
	*size = 0;
	
	char * content = get_json_content("./list_6502_opcodes/opcodes.json");
	if (content == NULL) {
		printf("Fatal Error: File \'./list_6502_opcodes/opcodes.json\' not found.\n");
		return FALSE;
	}
	
	cJSON *root = cJSON_Parse(content);
	if (root == NULL) {
		printf(">> Fatal Error: JSON_ROOT is NULL.\n");
		return_status = FALSE;
		goto goto_cleanup;
	}

	cJSON *opcodes = cJSON_GetObjectItemCaseSensitive(root, "OPCODES");
	if (! cJSON_IsArray(opcodes)) {
		printf(">> Fatal Error: JSON_OPCODES is not an array\n");
		return_status = FALSE;
		goto goto_cleanup;
	}

	cJSON *entry;
	cJSON_ArrayForEach(entry, opcodes) {
		
		cJSON *value = cJSON_GetObjectItemCaseSensitive(entry, "opcode");
		if (!cJSON_IsString(value)) {
			printf("Error: Invalid OPCODE field value.\n");
			return_status = FALSE;
			goto goto_cleanup;
		}
			
		cJSON *illegal = cJSON_GetObjectItemCaseSensitive(entry, "illegal");
		if (!cJSON_IsBool(illegal)) {
			printf("Error: Invalid Illegal field value.\n");
			return_status = FALSE;
			goto goto_cleanup;
		}
	
		if (! cJSON_IsTrue(illegal) ) {
			arr[(*size)++] = (uint8_t) strtol(value->valuestring, NULL, 0);
		}
	}

goto_cleanup:
	cJSON_Delete(root);
	free(content);
	return return_status;
}

uint8_t all_files_test(infra_raw * emulator, infra_raw * emulator_expected)
{
	
	uint8_t arr[0x100] = {0};
	uint16_t arr_size = 0;
	if (! get_json_legal_opcodes(arr, &arr_size) )
		return FALSE;

	uint16_t file_counter = 0;

	for (uint16_t i = 0; i < arr_size; i++) {
		
		char * filename = allocate_message_error(); // Preguiça de mudar o nome da função...
		snprintf(filename, ERROR_MESSAGE_SIZE, "./v1/%02x.json", arr[i]);

		printf(">> OPCODE %02x: ", arr[i]);

		char * content = get_json_content(filename);
		if (content == NULL) {
			printf("Error opening file.\n");
			free(filename);
			break;
		}

		printf("Selected.\n");

		cJSON *root = cJSON_Parse(content);
		if (root == NULL) {
			printf(">> Fatal Error: JSON_ROOT is NULL.\n");
			cJSON_Delete(root);
			free(content);
			free(filename);
			return FALSE;
			//exit(EXIT_FAILURE);
		}

		if (! init_test(&(emulator->cpu), &(emulator_expected->cpu), &(emulator->memory), &(emulator_expected->memory), &root) ) {
			cJSON_Delete(root);
			free(content);
			free(filename);
			return FALSE;
		}

		cJSON_Delete(root);
		free(content);
		free(filename);

		file_counter++;
		printf(">> [%d, %d]\n", file_counter, arr_size);
		printf("=-=-=-=-=-=-=-=-=-=-=-=\n");
	}

	return TRUE;
}

uint8_t single_file_test(const char * file, infra_raw * emulator, infra_raw * emulator_expected)
{
	char * filename = allocate_message_error(); // Preguiça de mudar o nome da função...
	snprintf(filename, ERROR_MESSAGE_SIZE, "./v1/%s.json", file);

	char * content = get_json_content(filename);
	if (content == NULL) {
		printf("Error opening file: %s\n", filename);
		free(filename);
		return FALSE;
	}

	cJSON *root = cJSON_Parse(content);
	if (root == NULL) {
		printf("Fatal Error: JSON_ROOT is NULL\n");
		cJSON_Delete(root);
		free(content);
		free(filename);
		return FALSE;
	}

	if (! init_test(&(emulator->cpu), &(emulator_expected->cpu), &(emulator->memory), &(emulator_expected->memory), &root) ) {
		cJSON_Delete(root);
		free(content);
		free(filename);
		return FALSE;
	}

	cJSON_Delete(root);
	free(content);
	free(filename);
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
	
	infra_raw emu = {
		.cpu	= p_cpu,
		.memory	= m_mem,
	};

	infra_raw emu_expected = {
		.cpu	= p_expected,
		.memory	= m_expected,
	};

	emu.cpu.bus		= (buses) { bus_raw_read, bus_raw_write, &emu };
	emu_expected.cpu.bus	= (buses) { bus_raw_read, bus_raw_write, &emu_expected };

	if ( strcmp(argv[1], "all") == 0 ) {
		if (! all_files_test(&emu, &emu_expected) ) {
			printf("Stoping all files test.\n");
			exit(EXIT_FAILURE);
		}
	} else {
		if (! single_file_test(argv[1], &emu, &emu_expected) ) {
			printf("Stoping single file test.\n");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
