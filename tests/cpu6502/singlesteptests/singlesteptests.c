#include "./../../../emulators/machine_test/machine_test.h"
#include "./../../../thirdparty/cJson/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TRUE			1
#define FALSE			0
#define FILE_LABEL		"[SingleStepTests]:"
#define FILE_LABEL_SUCCESS	"[SingleStepTests Success]:"
#define FILE_LABEL_ERROR	"[SingleStepTests Error]:"
#define TOTAL_TESTS		10000
#define ERROR_MESSAGE_SIZE	256

typedef struct {
	u16 passed;
	u16 failed;
} passed_failed_counter;

// Store the name of the test and what error occurred.
typedef struct {
	char * test_name;
	char * error;
} feedback;

extern u16 c_cycles[3 * 256];
extern u16 c_cycles_i;

// Others Function
static char	*allocate_message_error(void);
static void 	set_reg_error(const char *reg_name, uint64_t got, uint64_t expected, char **error_message);

// JSON ACCESS
static char 	*get_json_content(const char *filename);
static u8 	get_json_test_name(cJSON **test, char **test_name);
static u8 	get_json_legal_opcodes(u8 *arr, u16 *size);
static u8 	store_json_cycles(cJSON **test, u16 **cycles_expected, u16 *cycles_size);
static u8 	store_json_processor_content(struct cpu6502 *processor, cJSON **test, const char *field);
static u8 	store_json_memory_content(struct memory *m, cJSON **test, const char *field);

// Comparisons
static u8 	compare_processor(struct cpu6502 *p_cpu, struct cpu6502 *p_expected, char **error_message);
static u8 	compare_memory(struct memory *m_mem, struct memory *m_expected, char **error_message);
static u8 	compare_cycles(u16 *cycles, u16 size, u16 *cycles_expected, u16 size_expected, char **error_message);


// Init process
static void 	free_feedbacks(feedback *f, u16 size);
static void 	print_results(passed_failed_counter counter, feedback *feedbacks);
static u8 	init_test(struct cpu6502 *p_cpu, struct cpu6502 *p_expected, struct memory *m_mem, struct memory *m_expected, cJSON **root);

// Main
static u8 	all_files_test(struct machine_test *emulator, struct machine_test *emulator_expected);
static u8 	single_file_test(const char *file, struct machine_test *emulator, struct machine_test *emulator_expected);


void u8_decimal_to_binary_string(u8 v, char *b)
{
	u8 i = 0;
	u8 mask = 1;
	for (; i < 8; i++) {
		b[i] = ( (v & (mask << (7-i))) > 0 ) ? '1' : '0';
	}
	b[i] = '\0';

}

static char *allocate_message_error(void)
{
	return (char *) malloc(ERROR_MESSAGE_SIZE);
}

static char *get_json_content(const char *filename)
{
	FILE *file = fopen(filename, "r");

	if (file == NULL)
        	return NULL;

	// Get File Size
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = (char *) malloc(file_size + 1);
	fread(buffer, 1, file_size, file);
	buffer[file_size] = '\0';
	fclose(file);

	return buffer;
}

static u8 get_json_test_name(cJSON **test, char **test_name)
{
	cJSON *content = cJSON_GetObjectItemCaseSensitive(*test, "name");

	if (content == NULL) {
		fprintf(stderr, "%s get_json_test_name() got content pointing to NULL.\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	*test_name = content->valuestring;
	return TRUE;
}

static u8 get_json_legal_opcodes(u8 *arr, u16 *size)
{
	u8 return_status = TRUE;
	for (u16 i = 0; i < 0x100; i++)
		arr[i] = 0;
	*size = 0;

	char * content = get_json_content("./list_6502_opcodes/opcodes.json");
	if (content == NULL) {
		fprintf(stderr, "%s get_json_legal_opcodes() -> File \'./list_6502_opcodes/opcodes.json\' not found.\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *root = cJSON_Parse(content);
	if (root == NULL) {
		fprintf(stderr, "%s get_json_legal_opcode() got root pointing to NULL.\n", FILE_LABEL_ERROR);
		return_status = FALSE;
		goto goto_cleanup;
	}

	cJSON *opcodes = cJSON_GetObjectItemCaseSensitive(root, "OPCODES");
	if (! cJSON_IsArray(opcodes)) {
		fprintf(stderr, "%s get_json_legal_opcodes() -> OPCODES is not an array\n", FILE_LABEL_ERROR);
		return_status = FALSE;
		goto goto_cleanup;
	}

	cJSON *entry;
	cJSON_ArrayForEach(entry, opcodes) {

		cJSON *value = cJSON_GetObjectItemCaseSensitive(entry, "opcode");
		if (!cJSON_IsString(value)) {
			fprintf(stderr, "%s get_json_legal_opcodes() -> Invalid OPCODE field value (NOT A STRING).\n", FILE_LABEL_ERROR);
			return_status = FALSE;
			goto goto_cleanup;
		}

		cJSON *illegal = cJSON_GetObjectItemCaseSensitive(entry, "illegal");
		if (!cJSON_IsBool(illegal)) {
			fprintf(stderr, "%s get_json_legal_opcodes() -> Invalid illegal field value (NOT A BOOL).\n", FILE_LABEL_ERROR);
			return_status = FALSE;
			goto goto_cleanup;
		}

		if (! cJSON_IsTrue(illegal) ) {
			arr[(*size)++] = (u8) strtol(value->valuestring, NULL, 0);
		}
	}

goto_cleanup:
	cJSON_Delete(root);
	free(content);
	return return_status;
}

static u8 store_json_cycles(cJSON **test, u16 **cycles_expected, u16 *cycles_size)
{
	cJSON 	 *content = cJSON_GetObjectItemCaseSensitive(*test, "cycles");
	cJSON 	 *entry;

	u16 *c;
	u8  cycle_arr_size = 0;
	u16 i = 0;

	if (content == NULL) {
		fprintf(stderr, "%s store_json_cycles() got a NULL content.\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cycle_arr_size = (u8) cJSON_GetArraySize(content);
	c = (u16 *) malloc(3 * cycle_arr_size * sizeof(u16));

	cJSON_ArrayForEach(entry, content) {
		cJSON * addr 		= cJSON_GetArrayItem(entry, 0);
		cJSON * value		= cJSON_GetArrayItem(entry, 1);
		cJSON * read_write	= cJSON_GetArrayItem(entry, 2);

		if (!cJSON_IsNumber(addr) || !cJSON_IsNumber(value) || !cJSON_IsString(read_write)) {
			fprintf(stderr, "%s store_json_cycles() got invalid addr/value/read_write values.\n", FILE_LABEL_ERROR);
			free(c);
			return FALSE;
		}

		c[i++] = (u16) addr->valueint;
		c[i++] = (u16) value->valueint;
		c[i++] = (u16) ((strcmp((char *)read_write->valuestring, "write") == 0) ? CPU6502_FLAG_WRITE : CPU6502_FLAG_READ);
	}

	*cycles_expected = c;
	*cycles_size 	 = (u16) cycle_arr_size * 3;
	return TRUE;
}

static u8 store_json_processor_content(struct cpu6502 *processor, cJSON **test, const char *field)
{
	cJSON *content 	= cJSON_GetObjectItemCaseSensitive(*test, field);

	if (content == NULL) {
		fprintf(stderr, "%s store_json_processor_content() got a NULL content.\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *pc = cJSON_GetObjectItemCaseSensitive(content, "pc");
	if (!cJSON_IsNumber(pc)) {
		fprintf(stderr, "%s store_json_processor_content() got an invalid initial \'PC\' value (NOT NUMERIC).\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *s = cJSON_GetObjectItemCaseSensitive(content, "s");
	if (!cJSON_IsNumber(s)) {
		fprintf(stderr, "%s store_json_processor_content() got an invalid initial \'S\' value (NOT NUMERIC).\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *a = cJSON_GetObjectItemCaseSensitive(content, "a");
	if (!cJSON_IsNumber(a)) {
		fprintf(stderr, "%s store_json_processor_content() got an invalid initial \'A\' value (NOT NUMERIC).\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *x = cJSON_GetObjectItemCaseSensitive(content, "x");
	if (!cJSON_IsNumber(x)) {
		fprintf(stderr, "%s store_json_processor_content() got an invalid initial \'X\' value (NOT NUMERIC).\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *y = cJSON_GetObjectItemCaseSensitive(content, "y");
	if (!cJSON_IsNumber(y)) {
		fprintf(stderr, "%s store_json_processor_content() got an invalid initial \'Y\' value (NOT NUMERIC).\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *p = cJSON_GetObjectItemCaseSensitive(content, "p");
	if (!cJSON_IsNumber(p)) {
		fprintf(stderr, "%s store_json_processor_content() got an invalid initial \'P\' value (NOT NUMERIC).\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	processor->reg_pc	= (u16) 	pc->valueint;
	processor->reg_sp 	= (u8)  	s->valueint;
	processor->reg_a 	= (u8) 	a->valueint;
	processor->reg_x 	= (u8) 	x->valueint;
	processor->reg_y 	= (u8) 	y->valueint;
	processor->reg_p 	= (u8) 	p->valueint;
	processor->cycles	= 0;

	return TRUE;
}

static u8 store_json_memory_content(struct memory *m, cJSON **test, const char *field)
{
	cJSON *content 	= cJSON_GetObjectItemCaseSensitive(*test, field);

	memory_clean(m);

	if (content == NULL) {
		fprintf(stderr, "%s store_json_memory() got a NULL content.\n", FILE_LABEL_ERROR);
		return FALSE;
	}

	cJSON *ram = cJSON_GetObjectItemCaseSensitive(content, "ram");
	cJSON *entry;

	cJSON_ArrayForEach(entry, ram) {
		cJSON * addr 	= cJSON_GetArrayItem(entry, 0);
		cJSON * value	= cJSON_GetArrayItem(entry, 1);

		if (!cJSON_IsNumber(addr) || !cJSON_IsNumber(value)) {
			fprintf(stderr, "%s store_json_memory_content got a invalid memory addr/value (NOT NUMERIC).\n", FILE_LABEL_ERROR);
			return FALSE;
		}

		memory_write(m, (u16)addr->valueint, (u8)value->valueint);
	}

	return TRUE;
}

static void set_reg_error(const char *reg_name, uint64_t got, uint64_t expected, char **error_message)
{
	*error_message = allocate_message_error();
	char got_bin[9] = {0};
	char expected_bin[9] = {0};

	if (*error_message == NULL)
		return;
	if ( strcmp(reg_name,"P") == 0 ) {
		u8_decimal_to_binary_string((u8) got, got_bin);
		u8_decimal_to_binary_string((u8) expected, expected_bin);
		snprintf(*error_message, ERROR_MESSAGE_SIZE, "PROCESSOR REG_%s: got %lu (0b%s) expected %lu (0b%s).\n", reg_name, got, got_bin, expected, expected_bin);
		return;
	} else {
		snprintf(*error_message, ERROR_MESSAGE_SIZE, "PROCESSOR REG_%s: got %lu expected %lu.\n", reg_name, got, expected);
	}
	return;
}

static u8 compare_processor(struct cpu6502 *p_cpu, struct cpu6502 *p_expected, char **error_message)
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

static u8 compare_memory(struct memory *m_mem, struct memory *m_expected, char **error_message)
{
	u8 data_old, data_new;
	data_old = data_new = 0;

	for (u32 i = 0; i < m_mem->size; i++) {
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

static u8 compare_cycles(u16 *cycles, u16 size, u16 *cycles_expected, u16 size_expected, char **error_message)
{
	if (size != size_expected) {
		*error_message = allocate_message_error();
		snprintf(*error_message, ERROR_MESSAGE_SIZE, "CYCLES COUNTER: got %d expected %d.\n", size/3, size_expected/3);
		return FALSE;
	}

	for (u8 i = 0; i < (size_expected / 3); i++)
		for (u8 j = 0; j < 3; j++)
			if (cycles[j + (i * 3)] != cycles_expected[j + (i * 3)]) {
				*error_message = allocate_message_error();
				snprintf(*error_message, ERROR_MESSAGE_SIZE, "CYCLES: got [%d, %d, %d] expected [%d, %d, %d].\n", cycles[0 + (i * 3)], cycles[1 + (i * 3)], cycles[2 + (i * 3)], cycles_expected[0 + (i * 3)], cycles_expected[1 + (i * 3)], cycles_expected[2 + (i * 3)]);
				return FALSE;
		}
	return TRUE;
}

static void free_feedbacks(feedback *f, u16 size)
{
	for (u16 i = 0; i < size; i++)
		if (f[i].error != NULL)
			free(f[i].error);
}

static void print_results(passed_failed_counter counter, feedback *feedbacks)
{
	char flag;

	printf("%s Results: [%u/%u]\n", FILE_LABEL, counter.passed, TOTAL_TESTS);
	printf("%s Errors: ", FILE_LABEL);

	if (counter.failed == 0) {
		printf("NONE!!!\n");
	}
	else {
		printf("%d.\n", counter.failed);
		for (u16 i = 0; i < counter.failed; i++) {
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

static u8 init_test(struct cpu6502 *p_cpu, struct cpu6502 *p_expected, struct memory *m_mem, struct memory *m_expected, cJSON **root)
{
	cJSON			*test;
	passed_failed_counter	counter;
	feedback		feedbacks[TOTAL_TESTS];
	char			*test_name	= NULL;
	char			*error_message	= NULL;
	u16			*cycles_expected= NULL;
	u16			cycles_expected_size = 0;

	counter.passed = 0;
	counter.failed = 0;

	printf("%s Testing...\n", FILE_LABEL);
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

		cpu6502_opcode_execute(p_cpu);

		// The SingleStepTests does not simulate INTERRUPTS
		//cpu6502_interrupt_handle_request_if_any(p_cpu);

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

static u8 all_files_test(struct machine_test *emulator, struct machine_test *emulator_expected)
{
	u8  return_status = TRUE;
	u8  arr[U8_SIZE] = {0};
	u16 arr_size = 0;
	u16 file_counter = 0;

	if (! get_json_legal_opcodes(arr, &arr_size) )
		return FALSE;

	for (u16 i = 0; i < arr_size; i++) {

		char *filename = allocate_message_error();
		snprintf(filename, ERROR_MESSAGE_SIZE, "./v1/%02x.json", arr[i]);

		printf("%s OPCODE %02x selected for test.\n", FILE_LABEL, arr[i]);

		char *content = get_json_content(filename);
		if (content == NULL) {
			fprintf(stderr, "%s get_json_content() returned NULL instead of pointing to \'%s\'.\n", FILE_LABEL_ERROR, filename);
			return_status = FALSE;
			goto free_memory;
		}

		cJSON *root = cJSON_Parse(content);
		if (root == NULL) {
			fprintf(stderr, "%s cJSON_Parse() returned NULL\n", FILE_LABEL_ERROR);
			return_status = FALSE;
			goto free_memory;
		}

		if (! init_test(&(emulator->cpu), &(emulator_expected->cpu), &(emulator->ram), &(emulator_expected->ram), &root) ) {
			return_status = FALSE;
			goto free_memory;
		}

free_memory:
		if (root != NULL) cJSON_Delete(root);
		if (content != NULL) free(content);
		if (filename != NULL) free(filename);

		if (return_status == FALSE)
			return FALSE;

		file_counter++;
		printf("%s [%d/%d]\n", FILE_LABEL, file_counter, arr_size);
		printf("----------------------------------------------\n");
	}
	return TRUE;
}

static u8 single_file_test(const char *file, struct machine_test *emulator, struct machine_test *emulator_expected)
{
	u8	return_status = TRUE;
	char	*filename = allocate_message_error(); // Preguiça de mudar o nome da função...
	snprintf(filename, ERROR_MESSAGE_SIZE, "./v1/%s.json", file);
	char	*content = get_json_content(filename);

	if (content == NULL) {
		fprintf(stderr, "%s get_json_content returned NULL instead of pointing to file \'%s\'.\n", FILE_LABEL_ERROR, filename);
		return_status = FALSE;
		goto free_memory;
	}

	cJSON *root = cJSON_Parse(content);
	if (root == NULL) {
		fprintf(stderr, "%s cJSON_Parse() returned NULL\n", FILE_LABEL_ERROR);
		return_status = FALSE;
		goto free_memory;
	}

	if (! init_test(&(emulator->cpu), &(emulator_expected->cpu), &(emulator->ram), &(emulator_expected->ram), &root) ) {
		return_status = FALSE;
		goto free_memory;
	}

free_memory:
	if (root != NULL) cJSON_Delete(root);
	if (content != NULL) free(content);
	if (filename != NULL) free(filename);
	return return_status;
}

int main (int argc, char *argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "%s Specify the opcode in hexadecimal (without \'0x\').\n", FILE_LABEL_ERROR);
		fprintf(stderr, "\tPS: If you want to run all OPCPDES, just type \'all\' as an argument.\n");
		exit(EXIT_FAILURE);
	}

	u8 return_status = TRUE;
	struct cpu6502 p_cpu, p_expected;
	struct memory m_mem, m_expected;

	struct machine_test emu		= { .cpu = p_cpu, .ram = m_mem };
	struct machine_test emu_expected= { .cpu = p_expected, .ram = m_expected };

	if (! memory_alloc(&(emu.ram), MEMORY_RAM, U16_SIZE)) {
		fprintf(stderr, "%s memory_alloc() returned NULL.\n", FILE_LABEL_ERROR);
		exit(EXIT_FAILURE);
	}

	if (! memory_alloc(&(emu_expected.ram), MEMORY_RAM, U16_SIZE)) {
		fprintf(stderr, "%s memory_alloc() returned NULL.\n", FILE_LABEL_ERROR);
		exit(EXIT_FAILURE);
	}

	machine_test_prepare_cpu(&emu);
	machine_test_prepare_cpu(&emu_expected);

	cpu6502_init_manual(&(emu.cpu), 0x0000);
	cpu6502_init_manual(&(emu_expected.cpu), 0x0000);

	if ( strcmp(argv[1], "all") == 0 ) {
		if (! all_files_test(&emu, &emu_expected) ) {
			fprintf(stderr, "%s all_files_test() returned FALSE.\n", FILE_LABEL_ERROR);
			return_status = FALSE;
			goto free_memory;
		}
	} else {
		if (! single_file_test(argv[1], &emu, &emu_expected) ) {
			fprintf(stderr, "%s single_file_test() returned FALSE.\n", FILE_LABEL_ERROR);
			return_status = FALSE;
			goto free_memory;
		}
	}

free_memory:
	memory_free(&(emu.ram));
	memory_free(&(emu_expected.ram));
	if (return_status == FALSE)
		exit(EXIT_FAILURE);

	return 0;
}

