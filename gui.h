void gui_set_row_lengths(int new_num_rows, int *new_row_lengths);
void gui_set_action_proc(void (*proc)(int, int));
void gui_button_event(int button);

extern int gui_focus_row;
extern int gui_focus_col;
