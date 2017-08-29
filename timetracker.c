#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <form.h>

void print_new_activity(WINDOW *win);
void print_activities(WINDOW *win);
void start_new_activity();
void stop_new_activity();
void edit_new_activity();
void save_to_file(char *filepath);
void load_file(char *filepath);
char *create_data_files();

typedef struct activity
{
	time_t start_time;
	time_t end_time;
	char description[100];
} activity;

activity new_activity;
activity activities_list[100];

int main(int argc, char *argv[])
{
	char command;

    initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);

	strcpy(new_activity.description,"N/A");
	new_activity.start_time = time(NULL);

	load_file(create_data_files());
	
	while(command != 'q')
	{
		WINDOW *cur_act_win = newwin(5, COLS, 0, 0);
		wborder(cur_act_win, '|', '|', '-', '-', '+', '+', '+', '+');
		print_new_activity(cur_act_win);
		wrefresh(cur_act_win);

		WINDOW *past_act_win = newwin(LINES-5, COLS, 5, 0);
		wborder(past_act_win, '|', '|', ' ', '-', '|', '|', '+', '+');
		print_activities(past_act_win);
		wrefresh(past_act_win);
		
		command = wgetch(cur_act_win);
		switch(command)
		{
			case 's':
				start_new_activity();
				break;
			case 'p':
				stop_new_activity();
				break;
			case 'e':
				edit_new_activity();
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

void print_new_activity(WINDOW *win)
{
	time_t time_now = time(NULL);
	char start_time[80];

	strftime(start_time, sizeof(start_time), "%H:%M:%S", localtime(&new_activity.start_time));

	mvwprintw(win, 1, 1, "Current activity: %.*s", COLS-20, new_activity.description);
	mvwprintw(win, 2, 1, "Start time: %s", start_time);
	mvwprintw(win, 3, 1, "Duration: %ld", (time_now - new_activity.start_time)/60);
	
	wrefresh(win);
}

void print_activities(WINDOW *win)
{
	char start_time[80], end_time[80];

	mvwprintw(win, 0, 1, "Past activities:");
	for(int i = 0; activities_list[i].start_time; i++)
	{
		strftime(start_time, sizeof(start_time), "%H:%M:%S", localtime(&activities_list[i].start_time));
		strftime(end_time, sizeof(end_time), "%H:%M:%S", localtime(&activities_list[i].end_time));

		mvwprintw(win, 4 * i + 1, 1, "Start time: %s", start_time); 
		mvwprintw(win, 4 * i + 2, 1, "End time: %s", end_time);
		mvwprintw(win, 4 * i + 3, 1, "Activity: %.*s", COLS-13, activities_list[i].description);
	}
	
	wrefresh(win);
}

void start_new_activity()
{
	new_activity.start_time = time(NULL);
	memset(&new_activity.description[0], 0, sizeof(new_activity.description));
	edit_new_activity();
}

void stop_new_activity()
{
	new_activity.end_time = time(NULL);

	for(int i = 0; i < 100; i++)
	{
		if(!activities_list[i].start_time)
		{	
			activities_list[i] = new_activity;
			strcpy(new_activity.description, "N/A");
			break;
		}
	}
	
	save_to_file(create_data_files());
}

void edit_new_activity()
{
	WINDOW *win = newwin(10, 50, (LINES-10)/2, (COLS-50)/2);
	FORM *form;
	FIELD *field[2];
	int ch, rows, cols;

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
    while(ch != 10 && ch != 27)
	{
		ch = wgetch(win);
		switch(ch)
		{
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
				new_activity.description[strlen(new_activity.description)-1] = '\0';
				break;
			case 27:
				memset(&new_activity.description[0], 0, sizeof(new_activity.description));
				break;
			default:
				form_driver(form, ch);
				new_activity.description[strlen(new_activity.description)] = ch;
				break;
		}
	}
	curs_set(0);
	unpost_form(form);
	free_form(form);
	free_field(field[0]);
	delwin(win);
	new_activity.description[strcspn(new_activity.description, "\n")] = 0;
}

char *create_data_files()
{
	FILE *fp;
	time_t rawtime;
	struct tm *info;
	char timesheets_dir[80], date[11];
	static char	filepath[80];
	struct stat st = {0};
	
	snprintf(timesheets_dir, sizeof(timesheets_dir), "%s/Timesheets", getenv("HOME"));
	if(stat(timesheets_dir, &st) == -1) mkdir(timesheets_dir, 0700);
	
	time(&rawtime);
	strftime(date, sizeof(date), "%Y-%m-%d", localtime(&rawtime));
	snprintf(filepath, sizeof(filepath), "%s/timesheet-%s.txt", timesheets_dir, date);
	if(access(filepath, F_OK) == -1) 
	{
		fp = fopen(filepath, "w");
		fclose(fp);
	}

	return filepath;
}

void save_to_file(char *filepath)
{
	FILE *fp;

	fp = fopen(filepath, "w");
	for(int i = 0; activities_list[i].start_time; i++)
	{
		fprintf(fp, "%ld;%ld;%s\n", activities_list[i].start_time, activities_list[i].end_time, 
				activities_list[i].description);
	}
	fclose(fp);
}

void load_file(char *filepath)
{
	FILE *fp;
	int i;
	
	fp = fopen(filepath, "r");
	while (EOF != fscanf(fp, "%ld;%ld;%[^\n]", &activities_list[i].start_time, 
				&activities_list[i].end_time, activities_list[i].description)) i++;
	fclose(fp);
}
