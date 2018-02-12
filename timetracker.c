#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

typedef struct activity {
	char name[100];
	time_t start;
	time_t end;
} activity;

activity new, activities[100];

void print_new(WINDOW *win)
{
	time_t now = time(NULL);

	mvwprintw(win, LINES - 1, 0, "%.*s", COLS - 6, new.name);
	mvwprintw(win, LINES - 1, COLS - 5, "[%0.1f]",
		  difftime(now, new.start) / 3600);

	wrefresh(win);
}

void print_activities(WINDOW *win)
{
	ITEM **items;
	MENU *menu;
	int n;
	ITEM *cur_item;

	while (activities[n].start) n++;
	items = (ITEM **) calloc(n + 1, sizeof(ITEM *));
	for (int i = 0; i < n; ++i) {
		char name[100][100];
		char time[100][8];
		long dur = difftime(activities[i].end, activities[i].start);
		snprintf(name[i], 100, "%-*s", COLS - 7,  activities[i].name);
		snprintf(time[i], 8, "%0.1f    ", dur / 3600.0f);
		items[i] = new_item(name[i], time[i]);
	}
	items[n] = (ITEM *) NULL;
	menu = new_menu((ITEM **) items);
	set_menu_win(menu, win);
	set_menu_sub(menu, derwin(win, 0, 0, 4, 0));
	set_menu_format(menu, LINES - 6, 1);
	set_menu_mark(menu, 0);
	post_menu(menu);
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
	box(win, 0, 0);
	mvwprintw(win, 1, 1, "What are you doing: ");
	wmove(win, 2, 1);
	curs_set(1);
	while (ch != 10 && ch != 27) {
		ch = wgetch(win);
		switch (ch) {
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

void save(char *filepath)
{
	FILE *fp;

	fp = fopen(filepath, "w");
	for (int i = 0; activities[i].start; i++) {
		fprintf(fp, "%s|%ld|%ld\n", activities[i].name,
			activities[i].start, activities[i].end);
	}
	fclose(fp);
}

char *create_files()
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

	save(create_files());
}

void load_file(char *filepath)
{
	FILE *fp;
	char buf[100];
	int i = 0;
	char d[3][100];

	fp = fopen(filepath, "r");
	while (fgets (buf, 100, fp) != NULL) {
		if (sscanf(buf, "%99[^|]|%99[^|]|%99s",
					d[0], d[1], d[2]) == 3) {
			strcpy(activities[i].name, d[0]);
			activities[i].start = (time_t) atoi(d[1]);
			activities[i].end = (time_t) atoi(d[2]);
			i++;
		}
	}
	fclose(fp);
}

void print_archive(WINDOW *win)
{
	DIR *d;
        struct dirent *dir;
	int i = 1;

        d = opendir(".");
        if (d) {
            while ((dir = readdir(d)) != NULL) {
		mvwprintw(win, i + 3, 1, "%s", dir->d_name);
		i++;
            }
            closedir(d);
        }
}

WINDOW *print_main_window()
{
	WINDOW *win = newwin(LINES, COLS, 0, 0);

	time_t now = time(0);
	char buf[11];
	strftime(buf, 11, "%d.%m.%Y", localtime(&now));
	mvwprintw(win, 0, 0, "Today (%s)", buf);

	mvwhline(win, 1, 0, 0, COLS);
	mvwprintw(win, 2, 0, "Name");
	mvwprintw(win, 2, COLS - 6, "Time");
	mvwhline(win, 3, 0, 0, COLS);

	print_activities(win);
	mvwhline(win, LINES - 2, 0, 0, COLS);

	print_new(win);

	wrefresh(win);

	return win;
}

int main_window_controller()
{
	WINDOW *win;
	int cmd, out;

	while (cmd != 'q' && cmd != '2') {
		win = print_main_window();

		cmd = wgetch(win);
		switch(cmd) {
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
			save(create_files());
			break;
		case 'q':
			out = -1;
			break;
		case '2':
			out = 2;
			break;
		}

		delwin(win);
	}

	return out;
}

WINDOW *print_archive_window()
{
	WINDOW *win = newwin(LINES, COLS, 0, 0);

	mvwprintw(win, 0, 0, "Archive");
	mvwhline(win, 1, 0, 0, COLS);

	mvwprintw(win, 2, 0, "Day");
	mvwhline(win, 3, 0, 0, COLS);

	print_archive(win);
	mvwhline(win, LINES - 2, 0, 0, COLS);

	print_new(win);

	wrefresh(win);

	return win;
}

int archive_window_controller()
{
	WINDOW *win;
	int cmd, out;

	while (cmd != 'q' && cmd != '1') {
		win = print_archive_window();

		cmd = wgetch(win);
		switch (cmd) {
		case 'q':
			out = -1;
			break;
		case '1':
			out = 1;
			break;
		}

		delwin(win);
	}

	return out;
}

void window_controller()
{
	int i = 1;

	while (i != -1) {
		switch (i) {
		case 1:
			i = main_window_controller();
			break;
		case 2:
			i = archive_window_controller();
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);

	strcpy(new.name,"N/A");
	new.start = time(NULL);

	load_file(create_files());

	window_controller();
	endwin();

	return 0;
}
