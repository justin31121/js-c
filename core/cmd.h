#ifndef CMD_H
#define CMD_H

typedef unsigned long long int Cmd_u64;

#define u64 Cmd_u64

#ifndef CMD_DEF
#  define CMD_DEF static inline
#endif // CMD_DEF

#ifndef CMD_LOG
#  include <stdio.h>
#  define CMD_LOG(...) fprintf(stderr, __VA_ARGS__);
#endif // CMD_LOG

#define CMD_ERROR(...) CMD_LOG("ERROR: "); CMD_LOG(__VA_ARGS__); CMD_LOG("\n")

CMD_DEF int cmd_strcmp(char *a, char *b);
CMD_DEF char *cmd_next(int *argc, char ***argv);

typedef int (*Cmd_Func)(char *program, int argc, char **argv, void *userdata);

typedef struct{
  char *name;
  char *desc;
  Cmd_Func func;
}Cmd;

CMD_DEF void cmd_print_commands(Cmd *cmds, u64 cmds_len);
CMD_DEF int cmd_find(Cmd *cmds, u64 cmds_len, char *needle, u64 *index);
CMD_DEF int cmd_process(Cmd *cmds, u64 cmds_len, int argc, char **argv, void *userdata);

#ifdef CMD_IMPLEMENTATION

CMD_DEF int cmd_strcmp(char *_a, char *_b) {
  unsigned char *a = (unsigned char *)_a;
  unsigned char *b = (unsigned char *)_b;
  while(*a && *a == *b) ++a, ++b;
  return (*a > *b) - (*b > *a);
}

CMD_DEF char *cmd_next(int *argc, char ***argv) {
  if((*argc) == 0) return NULL;

  char *arg = (*argv)[0];
  (*argc) = (*argc) - 1;
  (*argv) = (*argv) + 1;

  return arg;
}

CMD_DEF void cmd_print_commands(Cmd *cmds, u64 cmds_len) {
  CMD_LOG("COMMANDS:\n");

  for(u64 i=0;i<cmds_len;i++) {
    CMD_LOG("     %s\n", cmds[i].name);
    CMD_LOG("          %s\n", cmds[i].desc);
  }

  CMD_LOG("     help\n");
  CMD_LOG("          Displays all available commands\n");
}

CMD_DEF int cmd_find(Cmd *cmds, u64 cmds_len, char *needle, u64 *index) {

  for(u64 i=0;i<cmds_len;i++) {
    if(cmd_strcmp(cmds[i].name, needle) == 0) {
      *index = i;
      return 1;
    }
  }

  return 0;
}

CMD_DEF int cmd_process(Cmd *cmds, u64 cmds_len, int argc, char **argv, void *userdata) {
  
  char *program = cmd_next(&argc, &argv);

  char *command = cmd_next(&argc, &argv);
  if(!command) {

    CMD_ERROR("Please provide a command");
    CMD_LOG("USAGE: %s <command> ...\n\n", program);
    cmd_print_commands(cmds, cmds_len);
    
    return 1;
  }

  if(cmd_strcmp("help", command) == 0) {
    char *subcommand = cmd_next(&argc, &argv);

    if(subcommand) {
      
      u64 index;
      if(cmd_find(cmds, cmds_len, subcommand, &index)) {
        CMD_LOG("%s - %s\n", cmds[index].name, cmds[index].desc);
				return 0;
			} else {
				CMD_ERROR("Unknown subcommand '%s'", subcommand);
				cmd_print_commands(cmds, cmds_len);
				return 1;
			}

		} else {
			cmd_print_commands(cmds, cmds_len);
			return 0;
		}
	}

	u64 index;
	if(cmd_find(cmds, cmds_len, command, &index)) {
		return cmds[index].func(program, argc, argv, userdata);  
	} else {
		CMD_ERROR("Unknown command '%s'", command);
		cmd_print_commands(cmds, cmds_len);
		return 1;
	}
}

#endif //CMD_IMPLEMENTATION

#undef u64

#endif // CMD_H
