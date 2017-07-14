#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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

	mvwprintw(win, 1, 1, "Current activity: %s", new_activity.description);
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
		mvwprintw(win, 4 * i + 3, 1, "Activity: %s", activities_list[i].description);
	}
	
	wrefresh(win);
}

void start_new_activity()
{
	new_activity.start_time = time(NULL);
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
}

void edit_new_activity()
{
	WINDOW *win = newwin(10, 50, (LINES-10)/2, (COLS-50)/2);
	wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
	mvwprintw(win, 1, 1, "What are you doing: ");

	echo();
	mvwgetstr(win, 2, 1, new_activity.description);
	noecho();
	
	new_activity.description[strcspn(new_activity.description, "\n")] = 0;
	wrefresh(win);
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
	while (EOF != fscanf(fp, "%ld;%ld;%s", &activities_list[i].start_time, &activities_list[i].end_time, 
				activities_list[i].description)) i++;
}
