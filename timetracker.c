#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <form.h>

typedef struct activity {
	time_t start;
	time_t end;
	char name[100];
} activity;

activity new;
activity activities[100];


void print_new(WINDOW *win)
{
	time_t time_now = time(NULL);
	char start[80];

	strftime(start, sizeof(start), "%H:%M:%S", localtime(&new.start));

	mvwprintw(win, 1, 1, "Current activity: %.*s", COLS - 20, new.name);
	mvwprintw(win, 2, 1, "Start time: %s", start);
	mvwprintw(win, 3, 1, "Duration: %ld", (time_now - new.start)/60);

	wrefresh(win);
}

void print_activities(WINDOW *win)
{
	char start[80], end[80];

	mvwprintw(win, 0, 1, "Past activities:");
	for (int i = 0; activities[i].start; i++) {
		strftime(start, sizeof(start), "%H:%M:%S",
			 localtime(&activities[i].start));
		strftime(end, sizeof(end), "%H:%M:%S",
			 localtime(&activities[i].end));

		mvwprintw(win, 4 * i + 1, 1, "Start time: %s", start);
		mvwprintw(win, 4 * i + 2, 1, "End time: %s", end);
		mvwprintw(win, 4 * i + 3, 1, "Activity: %.*s", COLS - 13,
			  activities[i].name);
	}

	wrefresh(win);
}

void edit_new()
{
	WINDOW *win = newwin(10, 50, (LINES - 10) / 2, (COLS - 50) / 2);
	FORM *form;
	FIELD *field[2];
	int ch, rows, cols;

	memset(new.name, 0, strlen(new.name));
	keypad(win, TRUE);
	field[0] = new_field(7, 48, 2, 1, 0, 0);
	field[1] = NULL;
	form = new_form(field);
	scale_form(form, &rows, &cols);
	set_form_win(form, win);
	post_form(form);
	wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
	mvwprintw(win, 1, 1, "What are you doing: ");
	wmove(win, 2, 1);
	curs_set(1);
	while (ch != 10 && ch != 27) {
		ch = wgetch(win);
		switch (ch) {
		/*
		case KEY_LEFT:
			form_driver(form, REQ_PREV_CHAR);
			break;
		case KEY_RIGHT:
			form_driver(form, REQ_NEXT_CHAR);
			break;
		case KEY_UP:
			form_driver(form, REQ_NEXT_LINE);
			break;
		case KEY_DOWN:
			form_driver(form, REQ_PREV_LINE);
			break;
		*/
		case 127:
			form_driver(form, REQ_PREV_CHAR);
			form_driver(form, REQ_DEL_CHAR);
			new.name[strlen(new.name) - 1] = '\0';
			break;
		case 27:
			memset(&new.name[0], 0, sizeof(new.name));
			break;
		default:
			form_driver(form, ch);
			new.name[strlen(new.name)] = ch;
			break;
		}
	}
	curs_set(0);
	unpost_form(form);
	free_form(form);
	free_field(field[0]);
	delwin(win);
	new.name[strcspn(new.name, "\n")] = 0;
}

void start_new()
{
	new.start = time(NULL);
	memset(&new.name[0], 0, sizeof(new.name));
	edit_new();
}

void save_to_file(char *filepath)
{
	FILE *fp;

	fp = fopen(filepath, "w");
	for (int i = 0; activities[i].start; i++) {
		fprintf(fp, "%ld;%ld;%s\n", activities[i].start,
			activities[i].end, activities[i].name);
	}
	fclose(fp);
}

char *create_data_files()
{
	FILE *fp;
	time_t rawtime;
	struct tm *info;
	char dir[80], date[11];
	static char filepath[80];
	struct stat st = {0};

	snprintf(dir, sizeof(dir), "%s/Timesheets", getenv("HOME"));
	if (stat(dir, &st) == -1) mkdir(dir, 0700);

	time(&rawtime);
	strftime(date, sizeof(date), "%Y-%m-%d", localtime(&rawtime));
	snprintf(filepath, sizeof(filepath), "%s/timesheet-%s.txt", dir, date);
	if (access(filepath, F_OK) == -1) {
		fp = fopen(filepath, "w");
		fclose(fp);
	}

	return filepath;
}

void stop_new()
{
	new.end = time(NULL);

	for(int i = 0; i < 100; i++) {
		if (!activities[i].start) {
			activities[i] = new;
			strcpy(new.name, "N/A");
			break;
		}
	}

	save_to_file(create_data_files());
}

void load_file(char *filepath)
{
	FILE *fp;
	int i;

	fp = fopen(filepath, "r");
	while (EOF != fscanf(fp, "%ld;%ld;%[^\n]", &activities[i].start,
			     &activities[i].end, activities[i].name))
		i++;
	fclose(fp);
}


int main(int argc, char *argv[])
{
	char command;

	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);

	strcpy(new.name,"N/A");
	new.start = time(NULL);

	load_file(create_data_files());

	while(command != 'q') {
		WINDOW *cur_act_win = newwin(5, COLS, 0, 0);
		wborder(cur_act_win, '|', '|', '-', '-', '+', '+', '+', '+');
		print_new(cur_act_win);
		wrefresh(cur_act_win);

		WINDOW *past_act_win = newwin(LINES-5, COLS, 5, 0);
		wborder(past_act_win, '|', '|', ' ', '-', '|', '|', '+', '+');
		print_activities(past_act_win);
		wrefresh(past_act_win);

		command = wgetch(cur_act_win);
		switch(command) {
		case 's':
			start_new();
			break;
		case 'p':
			stop_new();
			break;
		case 'e':
			edit_new();
			break;
		case 'v':
			save_to_file(create_data_files());
			break;
		case 'q':
			break;
		default :
			break;
		}
	}
    endwin();

    return 0;
}
