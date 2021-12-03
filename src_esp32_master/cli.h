void scroll_terminal(int lines);

void (*reset)(void) = 0;

byte get_clock_speed_cpu_mhz();

int cli_cmd_fbmem(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_fbinfo(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_hwinfo(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_help(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_lsdev(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_serial(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_df(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_mkdir(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_touch(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_ls(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_cd(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_sd(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_read_image_sequence(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_screenshot(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_cls(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_nop(char full_command[MAX_CLI_INPUT_LENGTH]);

int cli_cmd_err(char full_command[MAX_CLI_INPUT_LENGTH]);

void cli_output(cli_function function, char full_command[MAX_CLI_INPUT_LENGTH], VGA3BitI &vga_output);

void cli_nocmd();