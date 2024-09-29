#include <stdio.h>

#define FS_IMPLEMENTATION
#include <core/fs.h>

#define STR_IMPLEMENTATION
#include <core/str.h>

#define EBML_IMPLEMENTATION
#define EBML_TABLE \
	EBML_ENTRY(0x9C, MASTER, Job) \
	EBML_ENTRY(0xAE, UINT, exit_code) \
	EBML_ENTRY(0xD7, UTF8, command) \
	EBML_ENTRY(0xE7, UTF8, out) \
	EBML_ENTRY(0x86, UTF8, err)
#include <core/ebml.h>

#include <core/types.h>

#define ZH_IMPLEMENTATION
#include <core/zh.h>

#define CMD_IMPLEMENTATION
#include <core/cmd.h>

typedef struct {
	u8 exit_code;
	str command;
	str out;
	str err;
} Job;

#define Job_fmt "{ exit_code::%u, command::'"str_fmt"', stdout:'"str_fmt"', stderr::'"str_fmt"' }"
#define Job_arg(j) (j).exit_code, str_arg((j).command), str_arg((j).out), str_arg((j).err)

#define fs_unwrap(b) do{\
	Fs_Error __fs_unwrap_error = (b);\
	if(__fs_unwrap_error != FS_ERROR_NONE) {\
		return __fs_unwrap_error;\
	}\
}while(0)

// TODO: wrtten is never checked
Fs_Error job_serialize(Fs_File *f, Job *j) {

	u64 job_len = 0;

	Ebml_Vint vint_exit_code = ebml_to_vint(1);
	job_len += 1 + vint_exit_code.len + 1;

	Ebml_Vint vint_command;
	vint_command = ebml_to_vint(j->command.len);
	job_len += 1 + vint_command.len + j->command.len;

	Ebml_Vint vint_out = ebml_to_vint(j->out.len);
	vint_out = ebml_to_vint(j->out.len);
	job_len += 1 + vint_out.len + j->out.len;

	Ebml_Vint vint_err = ebml_to_vint(j->err.len);
	vint_err = ebml_to_vint(j->err.len);
	job_len += 1 + vint_err.len + j->err.len;

	Ebml_Vint vint_job = ebml_to_vint(job_len);

	u64 written;
	u8 id;

	id = EBML_ID_Job;
	fs_unwrap(fs_file_write(f, &id, 1, &written));
	fs_unwrap(fs_file_write(f, (u8 *) &vint_job.data, vint_job.len, &written));

	id = EBML_ID_exit_code;
	fs_unwrap(fs_file_write(f, &id, 1, &written));
	fs_unwrap(fs_file_write(f, (u8 *) &vint_exit_code.data, vint_exit_code.len, &written));
	fs_unwrap(fs_file_write(f, &j->exit_code, 1, &written));

	id = EBML_ID_command;
	fs_unwrap(fs_file_write(f, &id, 1, &written));
	fs_unwrap(fs_file_write(f, (u8 *) &vint_command.data, vint_command.len, &written));
	fs_unwrap(fs_file_write(f, j->command.data, j->command.len, &written));

	id = EBML_ID_out;
	fs_unwrap(fs_file_write(f, &id, 1, &written));
	fs_unwrap(fs_file_write(f, (u8 *) &vint_out.data, vint_out.len, &written));
	fs_unwrap(fs_file_write(f, j->out.data, j->out.len, &written));

	id = EBML_ID_err;
	fs_unwrap(fs_file_write(f, &id, 1, &written));
	fs_unwrap(fs_file_write(f, (u8 *) &vint_err.data, vint_err.len, &written));
	fs_unwrap(fs_file_write(f, j->err.data, j->err.len, &written));

	return FS_ERROR_NONE;
}

int job_deserialize(Ebml content, Job *j) {

	u8 found = 0;

	Ebml_Elem elem;
	u64 size;
	while(ebml_next(&content, &size, &elem)) {
		u8 *before = content.data - size;

		switch(elem.id) {
			case EBML_ID_exit_code:
				found |= 0x1;
				j->exit_code = *before;
				break;
			case EBML_ID_command:
				found |= 0x2;
				j->command = str_from(before, size);
				break;
			case EBML_ID_out:
				found |= 0x4;
				j->out = str_from(before, size);
				break;
			case EBML_ID_err:
				found |= 0x8;
				j->err = str_from(before, size);
				break;
			case EBML_ID_Job:
			default:
				return 0;
		}
	}

	return found == 0xf;

}

void dump_ebml(Ebml ebml) {

	Ebml_Elem elem;
	u64 size;
	while(ebml_next(&ebml, &size, &elem)) {
		printf("%s - %s - %llu\n", ebml_id_name(elem.id), ebml_type_name(elem.type), size);	
		u8 *before = ebml.data - size;

		switch(elem.type) {
			case EBML_TYPE_MASTER:
				dump_ebml(ebml_from(before, size));
				break;
			case EBML_TYPE_UINT:
				switch(size) {
					case 1:
						printf("\t%u\n", *before);
						break;
				}
				break;
			case EBML_TYPE_UTF8:
			case EBML_TYPE_STRING:
				printf("\t'%.*s'\n", (int) size, before);
				break;
			case EBML_TYPE_FLOAT:
			case EBML_TYPE_BINARY:
				break;
		}

	}

}

int capture(char *program, s32 argc, char **argv, void *userdata) {
	(void) userdata;

	if(argc < 2) {
		CMD_ERROR("To capture, provide atleast two arguments");
		CMD_LOG("USAGE: %s <commands.list> <commands.eml>", program);
		return 1;
	}
	char *filepath = argv[0];
	char *out_filepath = argv[1];

	str content;
	if(fs_slurp_filec(filepath, &content.data, &content.len) != FS_ERROR_NONE) {
		TODO();
	}
	str content_copy = content;

	Fs_File f;
	if(fs_file_wopenc(&f, out_filepath) != FS_ERROR_NONE) {
		TODO();
	}

	Zh zh = {0};
	Zh_str_builder sb_out = {0};
	Zh_str_builder sb_err = {0};
	s32 exit_code = 0;

	str line;
	while(str_chop_by(&content, "\n", &line)) {
		if(line.len > 0 && line.data[line.len - 1] == '\r') line.len--;

		zh.len = 0;
		sb_out.len = 0;
		sb_err.len = 0;

		zh_da_append_many(&zh, line.data, line.len);
		zh.len++;
		if(!zh_run(&zh, &exit_code, &sb_out, &sb_err)) {
			TODO();
		}

		printf("%.*s", (int) sb_out.len, sb_out.data);
		printf("%.*s", (int) sb_err.len, sb_err.data);

		Job job = {
			.exit_code = exit_code,
			.command = line,
			.out = str_from(sb_out.data, sb_out.len), 
			.err = str_from(sb_err.data, sb_err.len),
		};
		if(job_serialize(&f, &job) != FS_ERROR_NONE) {
			TODO();
		}

	}

	free(zh.data);
	free(sb_out.data);
	free(sb_err.data);

	fs_file_close(&f);

	FS_FREE(content_copy.data);

	zh_log(ZH_INFO, "Saved '%s'\n", out_filepath);

	return 0;
}

typedef enum {
	FIND_RESULT_FOUND,
	FIND_RESULT_ERROR,
	FIND_RESULT_NOT_FOUND,
} Find_Result;

Find_Result find_job(Ebml ebml, str command, Job *j) {

	Ebml_Elem elem;
	u64 size;
	while(ebml_next(&ebml, &size, &elem)) {
		if(elem.id != EBML_ID_Job) {
			return FIND_RESULT_ERROR;
		}

		if(!job_deserialize(ebml_from(ebml.data - size, size), j)) {
			return FIND_RESULT_ERROR;
		}

		if(str_eq(command, j->command)) {
			return FIND_RESULT_FOUND;
		}

	}

	return FIND_RESULT_NOT_FOUND;
}

int probe(char *program, s32 argc, char **argv, void *userdata) {
	(void) userdata;

	if(argc < 2) {
		CMD_ERROR("To capture, provide atleast two arguments");
		CMD_LOG("USAGE: %s <commands.list> <commands.eml>", program);
		return 1;
	}
	char *filepath = argv[0];
	char *out_filepath = argv[1];

	Ebml ebml;
	if(fs_slurp_filec(out_filepath, &ebml.data, &ebml.len) != FS_ERROR_NONE) {
		TODO();
	}

	str content;
	if(fs_slurp_filec(filepath, &content.data, &content.len) != FS_ERROR_NONE) {
		TODO();
	}

	Zh zh = {0};
	Zh_str_builder sb_out = {0};
	Zh_str_builder sb_err = {0};
	s32 exit_code = 0;

	str line;
	while(str_chop_by(&content, "\n", &line)) {
		if(line.len > 0 && line.data[line.len - 1] == '\r') line.len--;

		Job job;
		switch(find_job(ebml, line, &job)) {
			case FIND_RESULT_FOUND:
				break;
			case FIND_RESULT_ERROR:
			case FIND_RESULT_NOT_FOUND:
				TODO();
		}

		zh.len = 0;
		sb_out.len = 0;
		sb_err.len = 0;
		zh_da_append_many(&zh, line.data, line.len);

		zh.len++;
		if(!zh_run(&zh, &exit_code, &sb_out, &sb_err)) {
			TODO();
		}
		printf("%.*s", (int) sb_out.len, sb_out.data);
		printf("%.*s", (int) sb_err.len, sb_err.data);

		str out = str_from(sb_out.data, sb_out.len);
		str err = str_from(sb_err.data, sb_err.len);

		if(exit_code != job.exit_code) {
			TODO();
		}
		if(!str_eq(out, job.out)) {
			TODO();
		}
		if(!str_eq(err, job.err)) {
			TODO();
		}

	}

	free(zh.data);
	free(sb_out.data);
	free(sb_err.data);

	FS_FREE(ebml.data);

	return 0;
}

Cmd cmds[] = {
	{ "capture", 
		"TODO: capture description", 
		capture },
	{ "probe", 
		"TODO: probe description", 
		probe },
};

int main(s32 argc, char **argv) {
	return cmd_process(cmds, sizeof(cmds)/sizeof(cmds[0]), argc, argv, NULL);	
}


