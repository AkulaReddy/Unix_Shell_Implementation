#include<iostream>
#include<sys/utsname.h>
#include<string>
#include<string.h>
#include<stdio.h>
#include<unistd.h>
#include<pwd.h>
#include<vector>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <grp.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
using namespace std;

int saved_stdin  = dup(STDIN_FILENO);   
int saved_stdout = dup(STDOUT_FILENO); 

string cd_path=getenv("HOME");
string cd_prev;

vector<string> get_sys_cmds() {
    vector<string> sys_cmds;
    char* path = getenv("PATH");
    if (!path) return sys_cmds;

    char* path_copy = strdup(path);
    char* dir = strtok(path_copy, ":");

    while (dir) {
        DIR* d = opendir(dir);
        if (d) {
            struct dirent* entry;
            while ((entry = readdir(d)) != nullptr) {
                sys_cmds.push_back(entry->d_name);
            }
            closedir(d);
        }
        dir = strtok(nullptr, ":");
    }

    free(path_copy);
    return sys_cmds;
}

vector<string> syscmds = get_sys_cmds();

char* command_generator(const char* text, int state) {
    static size_t idx;
    static vector<string> matches;

    if (state == 0) {
        matches.clear();
        idx = 0;
        for (auto& cmd : syscmds) {
            if (cmd.find(text) == 0) {
                matches.push_back(cmd);
            }
        }
    }

    if (idx < matches.size()) {
        return strdup(matches[idx++].c_str());
    }
    return nullptr;
}


char** my_completion(const char* text, int start, int end) {
    (void)end; 
    if (start == 0) {
        return rl_completion_matches(text, command_generator);
    } else {
        return rl_completion_matches(text, rl_filename_completion_function);
    }
}

vector<char*> tokenize(char* input, const char* delim){
    char *token = strtok(input, delim);
    vector<char*> tokens;
    while (token != NULL) {
        tokens.push_back(token);
        token = strtok(NULL, delim); 
    }
    return tokens;
    
}

void file_perm(mode_t mode) {
    cout<<(S_ISDIR(mode) ? 'd' : '-')
        <<((mode & S_IRUSR) ? 'r' : '-')
        <<((mode & S_IWUSR) ? 'w' : '-')
        <<((mode & S_IXUSR) ? 'x' : '-')
        <<((mode & S_IRGRP) ? 'r' : '-')
        <<((mode & S_IWGRP) ? 'w' : '-')
        <<((mode & S_IXGRP) ? 'x' : '-')
        <<((mode & S_IROTH) ? 'r' : '-')
        <<((mode & S_IWOTH) ? 'w' : '-')
        <<((mode & S_IXOTH) ? 'x' : '-');
        cout << " ";
}

void ls_cmd(string dir,string file,int flags[]){
    if(!flags[0] && file[0]=='.') return;
    if (flags[1]) {
        struct stat st;
        string path = dir + "/" + file;
        if (stat(path.c_str(), &st) == -1) {
            perror("stat error");
            return;
        }
        file_perm(st.st_mode); 
        cout<< st.st_nlink << " ";
        cout<< (getpwuid(st.st_uid) ? getpwuid(st.st_uid)->pw_name : "?") << " ";
        cout<< (getgrgid(st.st_gid) ? getgrgid(st.st_gid)->gr_name : "?") << " ";
        cout<< st.st_size << " ";
        char time[80];
        strftime(time, sizeof(time), "%b %d %H:%M",localtime(&st.st_mtime));
        cout << time << " ";
        cout << file << "\n";
    } else {
        cout <<file << "\n";
    }
}


vector<int> execute_cmd(vector<char*>& tokens){
    vector<char* > tokenz;
    vector<int>fds;
    int fd0=-1,fd1=-1;
    char *token = tokens[0];
    const char* prev_tok="";
    for(int i=1;i<=tokens.size();i++) {
    if(strcmp(prev_tok, "<")==0){
        
        fd0 =open(token,O_RDONLY);
        if(fd0<0){perror("Given file not found");}
        
    }
    if(strcmp(prev_tok, ">")==0){
        
        fd1 =open(token,O_WRONLY | O_CREAT,0644);
        if(fd1<0){perror("Open failed");}
        
    }
    if(strcmp(prev_tok, ">>")==0){
        fd1 =open(token,O_WRONLY | O_CREAT | O_APPEND,0644);
        if(fd1<0){perror("Open failed");}
        //dup2(fd1,STDOUT_FILENO);
        //close(fd1a);
    }
    if(strcmp(token, ">")!=0 && strcmp(token, "<")!=0 && strcmp(token, ">>")!=0 && strcmp(prev_tok, ">")!=0 && strcmp(prev_tok, "<")!=0 && strcmp(prev_tok, ">>")!=0) tokenz.push_back(token);
    prev_tok=token;
    token=tokens[i];
    }

   fds.push_back(fd0);
   fds.push_back(fd1);
    
    tokens=tokenz;
    return fds;       
}

int exec_inbuilt(vector<char *> tokens){
            
            if(strcmp(tokens[0], "exit")==0) {

                exit(1);}
            if(strcmp(tokens[0], "cd")==0){
            
            string path;
            
            if (tokens.size()==1) {
                path = getenv("HOME");
            }
            else if(tokens.size()==2) {
                
                if(strcmp(tokens[1], "~")==0){
                    path = getenv("HOME");
                }
                else if(strcmp(tokens[1], "-")==0) {
                    cout<<cd_prev<<endl;
                    path=cd_prev;
                }
                else path=tokens[1];
            }
            else{
                cout<<"Invalid Arguments";
                return 0;
            }
           
            if(chdir(path.c_str())!=0) perror("cd error");
            else{
            //cout<<path<<endl;
            char curdir[1024];
            getcwd(curdir,sizeof(curdir));
            cd_prev=cd_path;
            cd_path=string(curdir);
            }
            return 0;
        }
        else if( strcmp(tokens[0], "pwd") == 0){
            char curdir[1024];
            getcwd(curdir,size(curdir));
            cout<<curdir<<endl;
            return 0;
        }
        else if( strcmp(tokens[0], "history") == 0){
            int num;
            if(tokens.size()==2) num=stoi(tokens[1]);
            else if(tokens.size()==1) num=10;
            else perror("invalid arguments");
            HIST_ENTRY **the_list = history_list();
            if (!the_list) return 0;
            int length = history_length; 
            num=min(num,length);
            for (int i=length-1; i>=length-num; i--) {
                cout<<the_list[i]->line<<endl;
            }
        }
        else if( strcmp(tokens[0], "echo") == 0){
            for(int i=1;i<tokens.size();i++){
                cout<<tokens[i]<<" ";
            }
            cout<<endl;
            return 0;
        }
        else if( strcmp(tokens[0], "pinfo") == 0){
            if(tokens.size()!=2) {cout<<"Invalid arguments";return 0;}
            try {
                int pid = stoi(tokens[1]);
        
                }
            catch (const invalid_argument &e) {
                    std::cout<<"Invalid argument";
                    return 0;
                }
            char proc_path[128],exe_path[128];
            snprintf(proc_path, sizeof(proc_path), "/proc/%s/stat", tokens[1]);
            snprintf(exe_path, sizeof(exe_path), "/proc/%s/exe", tokens[1]);
            int fd=open(proc_path,O_RDONLY);
            if(fd<0){
                perror("could not open");
                return 0;
            }
            char buf[4096];
            ssize_t n = read(fd, buf, sizeof(buf)-1);
            close(fd);
            if(n<=0){
                perror("could not read");
                return 0;
            }
            buf[n]='\0';

            int _pid;
            char dummy[256];
            char state;
            unsigned long mem_size;
            sscanf(buf, "%d %s %c", &_pid, dummy, &state);
            char *p = buf;
            for (int i = 0; i < 22; i++) {
                p = strchr(p+1, ' ');
            }
            sscanf(p, "%lu", &mem_size);
            char exePath[256];
            ssize_t len = readlink(exe_path, exePath, sizeof(exePath)-1);
            if (len == -1) {perror("error finding executable path"); return 0;}
            exePath[len] = '\0';
            cout<<"Process Status --"<<state<<endl;
            cout<<"memory--"<<mem_size<<endl;
            cout<<"Executable Path--"<<exePath<<endl;

        }
        else if( strcmp(tokens[0], "search") == 0){
            vector<string>files;
            if(tokens.size()!=2) {cout<<"error: Enter file to search"<<endl;return 0;}
            struct stat st;
            if (stat(".", &st) == -1) {
                perror("stat");
                return 0;
            }
            
            string search_res="False";
                
            DIR *d = opendir(".");
            if (!d) { perror("opendir"); return 0; }
            struct dirent *rd;
            while ((rd=readdir(d)) != NULL) {
                if(strcmp(tokens[1], rd->d_name)==0)search_res="True";
            }
            cout<<search_res<<endl;
            closedir(d);
            
        }
        else if( strcmp(tokens[0], "ls") == 0){
          
            int flags[2]={0,0};
            vector<string>dir;
            for(int i=1;i<tokens.size();i++){
               
                if(strcmp(tokens[i],"-a")==0) flags[0]=1;
                else if(strcmp(tokens[i],"-l")==0) flags[1]=1;
                else if(strcmp(tokens[i],"-al")==0 || (strcmp(tokens[i],"-la")==0)) {flags[0]=1;flags[1]=1;}
                else{
                    if (strcmp(tokens[i],"~")==0) tokens[i] = getenv("HOME");
                    dir.push_back(tokens[i]);  
                }
            }
            
            if (dir.empty()) dir.push_back(".");
            for (int i = 0; i < dir.size(); i++) {
               
                struct stat st;
                if (stat(dir[i].c_str(), &st) == -1) {
                    perror("stat");
                    continue;
                }
                if (S_ISDIR(st.st_mode)) {
                    if (dir.size() > 1) cout << dir[i] << ":\n";
                    DIR *d = opendir(dir[i].c_str());
                    if (!d) { perror("opendir"); continue; }
                    struct dirent *rd;
                    while ((rd=readdir(d)) != NULL) {
                        ls_cmd(dir[i], rd->d_name,flags);
                    }
                    closedir(d);
                } 
                else {
                    ls_cmd(".", dir[i],flags);
                }
                if (i < dir.size() - 1) cout << "\n";
            }
        }
        else  return 1;
            
        return 0;
}
int fg_id=-1;
void si_han(int sig){
    if(fg_id>0){
        kill(fg_id,SIGINT);
    }
}
void st_han(int sig){
    if(fg_id>0){
        kill(fg_id,SIGTSTP);
    }
}


int main() { 
    signal(SIGINT,si_han);
    signal(SIGTSTP,st_han);
    signal(SIGINT,  SIG_IGN); 
    signal(SIGTSTP, SIG_IGN);   
    signal(SIGQUIT, SIG_IGN);   
    signal(SIGTTIN, SIG_IGN);   
    signal(SIGTTOU, SIG_IGN);   
    signal(SIGCHLD, SIG_DFL); 
    
    rl_attempted_completion_function = my_completion;
    
    char *hostname; size_t len;
    struct utsname buf;
    int e = uname(&buf);
    hostname=buf.nodename;
    if(e==-1){
        perror("host name error");
    }

    char homed[1024];
    getcwd(homed,sizeof(homed));
    string hd=string(homed);

    using_history();
    char path_his[1024];
    snprintf(path_his, sizeof(path_his), "%s/%s", hd.c_str(), ".shell_his");
    read_history(path_his);

    char *user_name =getpwuid(getuid())->pw_name;
    string disp=string(user_name)+"@"+string(hostname)+":";

    while (true) {
        char cwd[1024];
        //const char* hd;
        getcwd(cwd,sizeof(cwd));
        //hd=getpwuid(getuid())->pw_dir;
        string curwd=string(cwd);
        
        if(curwd.rfind(hd,0)==0){
            curwd.replace(0,hd.size(),"~");
        }
        string prompt=disp+curwd+">";
        char* input = readline(prompt.c_str());
        if (!input) break; 

        if (*input) {
            
            add_history(input);
            append_history(1,path_his);
            history_truncate_file(path_his,20);
        }

       

        // tokenize wrt ; -> to get vector of char* type strings: these may contain | or ><>> each
        const char* delim=";";
        vector<char*>cmd_ls=tokenize(input,delim);

        for(auto cmd:cmd_ls){
            //From now on code for exactly one cmd which may contain | and <>>>
            const char* delim="|";
            vector<char*>piped_cmd_ls=tokenize(cmd,delim);
            int in_fd=0;
            pid_t rpid;
            if(piped_cmd_ls.size()==1){

            int f;
            vector<char*>tokenz = tokenize(piped_cmd_ls[0]," \n\t");
            char* token=tokenz[0];
            const char* prev_tok="";
            vector<char*>tokens;
            int fd0,fd1,fd1a;
            for(int i=1;i<=tokenz.size();i++) {
            if(strcmp(prev_tok, "<")==0){
                //cout<<token<<endl;
                fd0 =open(token,O_RDONLY);
                if(fd0<0){perror("Given file not found");}
                dup2(fd0,STDIN_FILENO);
                close(fd0);
            }
            if(strcmp(prev_tok, ">")==0){
                //cout<<token<<endl;
                fd1 =open(token,O_WRONLY | O_CREAT,0644);
                if(fd1<0){perror("Open failed");}
                dup2(fd1,STDOUT_FILENO);
                close(fd1);
            }
            if(strcmp(prev_tok, ">>")==0){
                fd1a =open(token,O_WRONLY | O_CREAT | O_APPEND,0644);
                if(fd1a<0){perror("Open failed");}
                dup2(fd1a,STDOUT_FILENO);
                close(fd1a);
            }
            if(strcmp(token, ">")!=0 && strcmp(token, "<")!=0 && strcmp(token, ">>")!=0 && strcmp(prev_tok, ">")!=0 && strcmp(prev_tok, "<")!=0 && strcmp(prev_tok, ">>")!=0) tokens.push_back(token);
            prev_tok=token;
            token=tokenz[i];
            }
            if(tokens.empty()) continue;

            f= exec_inbuilt(tokens);
            dup2(saved_stdin, STDIN_FILENO);
            dup2(saved_stdout, STDOUT_FILENO);
           
            if(f){
            pid_t pid = fork();
            int status,fore_flag=1;
            if(strcmp(tokens.back(),"&")==0){
                    fore_flag=0;
                    tokens.pop_back();
                }
            
            if (pid == 0) {
                vector<int>fds= execute_cmd(tokens);
                dup2(fds[0],STDIN_FILENO);
                dup2(fds[1],STDOUT_FILENO);

                tokens.push_back(nullptr);
                int e=execvp(tokens[0],tokens.data());
                if(e==-1) perror("command not found");
                close(fds[0]);
                close(fds[1]);
            }
                 
            else{

                if(fore_flag){
                fg_id=pid;
                do {
                    int rpid = waitpid(pid, &status, WUNTRACED);
                    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                    fg_id=-1;
                }
                else{
                    cout<<"pid: "<<pid<<endl;
                }
                
            }
            
            }
            continue;
            }
            else{
            int in_fd=0;
            for(int i=0;i<piped_cmd_ls.size();i++){
                vector<char*>tokens = tokenize(piped_cmd_ls[i]," \n\t");

                int fore_flag=1;
                if(strcmp(tokens.back(),"&")==0){
                    fore_flag=0;
                    tokens.pop_back();
                }
                int fd[2];
                if (i < piped_cmd_ls.size() - 1) pipe(fd); 

                pid_t pid = fork();
                if (pid == 0) {
                    vector<int>fds= execute_cmd(tokens);
                    if (fds[0] != -1) {
                        dup2(fds[0], STDIN_FILENO);
                        close(fds[0]);
                    }
                    if (fds[1] != -1) {
                        dup2(fds[1], STDOUT_FILENO);
                        in_fd=fds[1];
                        close(fds[1]);
                    }
                   
                    if (in_fd != 0) {
                        dup2(in_fd, 0); 
                        close(in_fd);
                    }
                    if (i < piped_cmd_ls.size() - 1) {
                        dup2(fd[1], 1); 
                       
                    }
                    close(fd[0]);
                    close(fd[1]);
                    
                    tokens.push_back(nullptr);
                    int e=execvp(tokens[0],tokens.data());
                    if(e==-1) perror("command not found");
                    
                } 
                else {
                    if (in_fd != 0) close(in_fd);
                    if (i < piped_cmd_ls.size() - 1) {
                        close(fd[1]);
                        in_fd = fd[0];
                    }
                    if(fore_flag){
                        int status;
                    
                        waitpid(pid, &status, WUNTRACED);   

                    }
                    else{
                        cout<<"pid: "<<pid<<endl;
                    }
                }
            }
            }
    
        }
        
        free(input);
    }

    return 0;

}
