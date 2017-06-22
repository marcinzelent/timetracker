#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void print_new_activity();
void start_new_activity();
void stop_new_activity();
void edit_new_activity();
void print_activities();
void save_to_file();
void load_file();

typedef struct activities
{
	time_t start_time;
	time_t end_time;
	char description[100];
} activity;

activity new_activity;
activity activities_list[100];

int main()
{
	strcpy(new_activity.description,"N/A");
	new_activity.start_time = time(NULL);
	load_file();
	char command;

	while(command != 'q')
	{
		system("clear");
		print_new_activity();
		print_activities();
		printf("\n\n[s]tart/sto[p]/[e]dit new activity | sa[v]e to file | [q]uit: ");		
		fgets(&command, 256, stdin);
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
				save_to_file();
				break;

			case 'q':
				break;

			default :
				break;
		}
	}
	return 0;
}

void print_new_activity()
{
	time_t time_now = time(NULL);
	char buf[80];

	strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&new_activity.start_time));

	printf("=================================================="
			"==================================================\n"
			"Current activity: %s\nStart time: %s\nDuration: %ld mins.\n"
			"=================================================="
			"==================================================\n",
			new_activity.description, buf, (time_now - new_activity.start_time)/60);
}

void print_activities()
{
	char buf1[80], buf2[80];

	printf("\nPast activities:\n"
			"--------------------------------------------------"
			"--------------------------------------------------"); 
	for(int i = 0; activities_list[i].start_time; i++)
	{
		strftime(buf1, sizeof(buf1), "%H:%M:%S", localtime(&activities_list[i].start_time));
		strftime(buf2, sizeof(buf2), "%H:%M:%S", localtime(&activities_list[i].end_time));

		printf("\nStart time: %s\nEnd time: %s\nActivity: %s\n"
				"--------------------------------------------------"
				"--------------------------------------------------", 
				buf1, buf2, activities_list[i].description);
	}
}

void start_new_activity()
{
	new_activity.start_time = time(NULL);

	printf("\nWhat are you doing: ");
	fgets(new_activity.description, 256, stdin);
	new_activity.description[strcspn(new_activity.description, "\n")] = 0;
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
	printf("\nWhat are you doing: ");
	fgets(new_activity.description, 256, stdin);
	new_activity.description[strcspn(new_activity.description, "\n")] = 0;
}

void save_to_file()
{
	FILE *fp;
	time_t rawtime;
	struct tm *info;
	char buffer[80], filename[80];

	time(&rawtime);
	strftime(buffer, 80, "%Y-%m-%d", localtime(&rawtime));
	
	snprintf(filename, sizeof(filename), "%s/Timesheets/timesheet-%s.txt", getenv("HOME"), buffer);
	fp = fopen(filename, "w");
	
	for(int i = 0; activities_list[i].start_time; i++)
	{
		fprintf(fp, "%ld;%ld;%s\n", activities_list[i].start_time, activities_list[i].end_time, 
				activities_list[i].description);
	}
	fclose(fp);
}

void load_file()
{
	FILE *fp;
	time_t rawtime;
	struct tm *info;
	char buffer[80], filename[80], timesheets_dir[80];
	int i = 0;
	struct stat st = {0};

	snprintf(timesheets_dir, sizeof(timesheets_dir), "%s/Timesheets/", getenv("HOME"));

	if (stat(timesheets_dir, &st) == -1) 
	{
		mkdir(timesheets_dir, 0700);
	}

	time(&rawtime);
	strftime(buffer, 80, "%Y-%m-%d", localtime(&rawtime));
	
    snprintf(filename, sizeof(filename), "%s/Timesheets/timesheet-%s.txt", getenv("HOME"), buffer);
    fp = fopen(filename, "r");
	if(fp == NULL) fp = fopen(filename, "w");
	while (EOF != fscanf(fp, "%ld;%ld;%s", &activities_list[i].start_time, &activities_list[i].end_time, 
				activities_list[i].description)) i++;
}
