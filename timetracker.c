#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>

void print_current_activity();
void start_new_activity();
void stop_current_activity();
void print_activities();
void save_to_file();

typedef struct activities
{
	time_t start_time;
	time_t end_time;
	char description[100];
} activity;

activity current_activity;
activity activities_list[100];

int main()
{

	strcpy(current_activity.description,"N/A\n");
	char command;

	while(command != 'q')
	{
		system("clear");
		print_current_activity();
		print_activities();
		printf("\n\n[s]tart tracking new activity | sto[p] current activity | sa[v]e to file | [q]uit: ");		
		fgets(&command, 256, stdin);
		switch(command)
		{
			case 's':
				start_new_activity();
				break;

			case 'p':
				stop_current_activity();
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

void print_current_activity()
{
	time_t time_now = time(NULL);

	printf("=================================================="
			"==================================================\n"
			"Current activity: %sStart time: %sDuration: %ld mins.\n"
			"=================================================="
			"==================================================\n",
			current_activity.description, ctime(&current_activity.start_time), 
			(time_now - current_activity.start_time)/60);
}

void print_activities()
{
	struct tm ts1, ts2;
	char buf1[80], buf2[80];

	printf("\nPast activities:\n"
			"--------------------------------------------------"
			"--------------------------------------------------"); 
	for(int i = 0; activities_list[i].start_time; i++)
	{
		ts1 = *localtime(&activities_list[i].start_time);
		ts2 = *localtime(&activities_list[i].end_time);

		strftime(buf1, sizeof(buf1), "%a %Y-%m-%d %H:%M:%S %Z", &ts1);
		strftime(buf2, sizeof(buf2), "%a %Y-%m-%d %H:%M:%S %Z", &ts2);

		printf("\nStart time: %s\nEnd time: %s\nActivity: %s"
				"--------------------------------------------------"
				"--------------------------------------------------", 
				buf1, buf2, activities_list[i].description);
	}
}

void start_new_activity()
{
	current_activity.start_time = time(NULL);

	printf("\nWhat are you doing: ");
	fgets(current_activity.description, 256, stdin);
}

void stop_current_activity()
{
	current_activity.end_time = time(NULL);

	for(int i = 0; i < 100; i++)
	{
		if(!activities_list[i].start_time)
		{	
			activities_list[i] = current_activity;
			strcpy(current_activity.description, "N/A\n");
			break;
		}
	}

}

void save_to_file()
{
	FILE *fp;
	time_t rawtime;
    struct tm *info;
    char buffer[80];

    time(&rawtime);

    strftime(buffer, 80, "%Y-%m-%d", localtime(&rawtime));

	char filename[80];
	snprintf(filename, sizeof(filename), "%s/Timesheets/timesheet-%s.txt", getenv("HOME"), buffer);

	fp = fopen(filename, "w");
	
	for(int i = 0; activities_list[i].start_time; i++)
	{
		fprintf(fp, "%ld\n%ld\n%s\n", activities_list[i].start_time, activities_list[i].end_time, 
				activities_list[i].description);
	}

	fclose(fp);
}

void load_file()
{
	FILE *fp;

	snprintf(filename, sizeof(filename), "%s/Timesheets/timesheet-%s.txt", getenv("HOME"), buffer);

	fp = fopen(filename, "r");

	char buffer[255];
	
	while (fgets(buffer, sizeof buffer, stream) != NULL)	
	{
		
	}
	if (feof(stream)) 
	{

	}
	else
	{

	}	
}
