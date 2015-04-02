#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <omp.h>

#define GREEN   "\033[32m"      /* Green */
#define RESET   "\033[0m"

//compile command: g++ md5_omp.cpp -o md5_omp -std=c++11 -fopenmp
//path : /home/glassfish4/glassfish/lib/schemas

using namespace std;

struct file{
	string md5;
	string name;
};

//list files in specific directory
vector<string> getdir (string dir, vector<string> &files)
{
	DIR *dp;
	struct dirent *dirp;
	
	if((dp = opendir(dir.c_str())) == NULL) 
	{
		cout << "Error(" << errno << ") opening " << dir << endl;
		return files;
	}

	while ((dirp = readdir(dp)) != NULL) 
	{

			if(dirp->d_type == DT_REG)
			{
				files.push_back(dir + string(dirp->d_name));
				//cout << "file add " + dir + string(dirp->d_name) << endl;
			}
			else if (dirp->d_type == DT_DIR)
			{
				string fname = dirp->d_name;
				if( fname != "." && fname != "..")
					{
						getdir(dir + string(dirp->d_name) + "/",files);
					}
		}
	}	
	
	closedir(dp);
	//cout << files.size() << endl;
	return files;
}

//run specific command using command line
string exec(string cmd) 
{
	FILE* pipe = popen(cmd.c_str(), "r");
	
    if (!pipe) return "ERROR";
	
	char buffer[128];
    
	string result = "";
    
	while(!feof(pipe))
	{
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    
	pclose(pipe);
    
	return result;
}

//split exec() output to get md5
string split(string input)
{

	vector<string> results;
	stringstream s(input);
	
	while(!s.eof()) 
	{
		string tmp;
		s >> tmp;
		results.push_back(tmp);
	}
	
	return results.front();
}


int main(int argc, char *argv[])
{
	omp_set_num_threads(4);
	double end_time;
	double start_time;
	
	string dir = argv[1]; 
	//string dir = string("/home/pywapi-0.3.8/examples/");
	vector<string> files;
	vector<file> md5;
	string cmd = "md5sum ";
	
    #pragma omp parallel
	{
		#pragma omp master
		{
			getdir(dir,files);
		}
		
		 //inicjalizacja licznika czasu
		start_time = omp_get_wtime();
		#pragma omp for schedule(auto)
		for (int i = 0;i < files.size();i++) 
			{
					// << files[i] << endl;
					cout << "MD5 for:  " + files[i] +": "+ split(exec(cmd+ files[i])) << endl;
					md5.push_back({split(exec(cmd + files[i])),files[i]});
			}
		end_time = omp_get_wtime();
		
		#pragma omp master
		{
			for (unsigned int i = 0 ;i < md5.size(); i++)
				{
					for (unsigned int j = 0 ;j < md5.size(); j++)
					{
						if(md5[i].name != md5[j].name & md5[i].md5 == md5[j].md5)
						{
							cout << GREEN << "Equal MD5: " + md5[i].name +" "+md5[j].name << RESET << endl;
							md5.erase(md5.begin() + i);
						}
					}
				}
			}
		}
		cout << "Czas wykonywania obliczen wynosi: " << end_time - start_time << endl;
	return 0;
}
