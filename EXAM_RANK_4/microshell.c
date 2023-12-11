
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int print_fd(char *message, char *data, int fd)
{
    int i;
    i = 0;
    while (message[i])
    {
        write(fd, &message[i], sizeof(message[i]));
        i++;
    }
    if (data)
    {
        i = 0;
        while (data[i])
        {
            write(fd, &data[i], sizeof(data[i]));
            i++;
        }
    }
    write(fd, "\n", 1);
    return (1);
}

void exec_cd(char **argv, int j)
{
    if (j != 2)
        print_fd("error: cd: bad arguments", 0, 2);
    else if (chdir(argv[1]) == -1)
        print_fd("error: cd: cannot change directory to ", argv[1], 2);
}

void exec_cmd(char **argv, char **env, int j)
{
    int fd[2];
    int pid;
    int status;
    int has_pipe = (argv[j]) && (strcmp(argv[j], "|") == 0);

    if ((has_pipe) && (pipe(fd) == -1)) // error
    {
        print_fd("error: fatal", 0, 2);
        return;
    }
    pid = fork();
    if (pid == 0)
    {
        argv[j] = 0;
        if (has_pipe &&
            (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
        {
            print_fd("error: fatal", 0, 2);
            return;
        }
        execve(*argv, argv, env);
        print_fd("error: cannot execute ", argv[0], 2);
        return;
    }
    waitpid(pid, &status, 0);
    if (has_pipe &&
            (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
        print_fd("error: fatal", NULL, 2);
}

int main(int argc, char *argv[], char **env)
{
    int j;
    int i = 1;
    if (argc > 1)
    {
        while (i < argc)
        {
            j = 0;
            while (argv[i + j] && (strcmp(argv[i + j], "|")) && (strcmp(argv[i + j], ";")))
                j++;
            if (strcmp(argv[i], "cd") == 0)
                exec_cd(&argv[i], j);
            else if (j)
                exec_cmd(&argv[i], env, j);
            i += j + 1;
        }
    }
    return 0;
}