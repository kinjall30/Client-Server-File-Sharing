// Section: 3
// ID: 110100419 - Ayush Rana
// ID: 110095279 - Kinjal Prajapati
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 4444
#define BUFFER_SIZE 4096 // Buffer size for receiving data from clients
#define MAX_CLIENTS 12   // Maximum number of clients (including server and mirror)
#define MAX_PATH_LENGTH 5000
#define MAX_ARGS 7
#define MAX_CMD_LEN 2048
#define MAX_COMMAND_LENGTH 1024

char fileBuffer[1024] = {0};
void kiay_clientArgHandleFun(int client_socket);

// FGETS
void kiay_findDir(char *dir_path, char **files, int nofiles, char *complete_files_path)
{
    DIR *dir = opendir(dir_path); // Open the directory specified by given pth
    if (!dir)
    {
        printf("Error opening directory"); // Print an error message if opening the directory fails
        exit(1);                           // Exit the program with a failure status
    }
    struct dirent *entry;        // Declare a pointer to a dirent structure
    char fpath[MAX_PATH_LENGTH]; // Declare a character array to store the complete file path

    // Loop through each entry in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR) // Check if the entry is a directory
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue; // Skip the "." and ".." entries
            }
            sprintf(fpath, "%s/%s", dir_path, entry->d_name); // Create the complete path to the subdirectory
            // Recursively call the function for the subdirectory
            kiay_findDir(fpath, files, nofiles, complete_files_path);
        }
        else if (entry->d_type == DT_REG) // Check if the entry is a regular file
        {
            for (int i = 0; i < nofiles; i++) // Loop through the list of files to search for
            {
                if (strcmp(files[i], entry->d_name) == 0) // Compare the file name with the current entry
                {
                    char fpath[MAX_PATH_LENGTH];
                    sprintf(fpath, "%s/%s", dir_path, entry->d_name); // Create the complete path to the file
                    // Append the file path to the complete_files_path string
                    sprintf(complete_files_path, "%s%s%s", complete_files_path, fpath, " ");
                }
            }
        }
    }

    closedir(dir); // Close the directory stream
}

// Func for TARFGETZ
void kiay_tarfgetz(const char *sourceDir, const char *targetTar, const char *minSize, const char *maxSize)
{

    char command[MAX_CMD_LEN]; // Declare a character array to store the command
    // Build the command using the provided parameters and store it in the 'command' array
    snprintf(command, MAX_CMD_LEN, "find %s -type f -size +%sc -a -size -%sc -print0 | tar czvf %s --null -T - >/dev/null 2>&1",
             sourceDir, minSize, maxSize, targetTar);

    // Execute the command using the system function and store the result
    int result = system(command);
    if (result == -1)
    {
        printf("Error executing command!"); // Print an error message if the command execution fails
        exit(EXIT_FAILURE);                 // Exit the program with a failure status
    }
}

// func for FILESRCH
void kiay_filesrch(int client_socket, char *filename, char *path, int *found)
{
    DIR *dir;              // Declare a pointer to a directory stream
    struct dirent *dp;     // Declare a pointer to a dirent structure
    struct stat st;        // Declare a structure to store file information
    char buf[BUFFER_SIZE]; // Declare a buffer to store paths and filenames

    dir = opendir(path); // Open the directory specified by the 'path'
    if (dir == NULL)
    {
        printf("opendir"); // Print an error message if opening the directory fails
        return;
    }

    while ((dp = readdir(dir)) != NULL) // Loop through each entry in the directory
    {

        if (strcmp(dp->d_name, filename) == 0) // Compare the entry's name with the target filename
        {

            sprintf(buf, "%s/%s", path, filename); // Create the full path to the target file

            if (stat(buf, &st) == 0) // Retrieve file information (size, modification time)
            {

                printf("File Path:%s\nFilename:%s\nFile Size:%ld\nCreated At:%s\n", path, filename, st.st_size, ctime(&st.st_mtime)); // Print file information
                char message[1000];
                memset(message, 0, sizeof(message));
                sprintf(message, "File Path:%s\nFilename:%s\nFile Size:%ld\nCreate At:%s\n", path, filename, st.st_size, ctime(&st.st_mtime)); // Store file information in a message
                message[strlen(message)] = '\0';
                if (send(client_socket, message, strlen(message), 0) < 0) // Send the message to the client
                {
                    printf("sending file fails"); // Print an error message if sending fails
                    exit(EXIT_FAILURE);
                }
                memset(message, 0, sizeof(message)); // Clear the message buffer
                *found = 1;                          // Set the 'found' flag to indicate the file was found
                break;                               // Exit the loop since the file was found
            }
        }

        if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) // Check if the entry is a subdirectory
        {
            sprintf(buf, "%s/%s", path, dp->d_name);            // Create the full path to the subdirectory
            kiay_filesrch(client_socket, filename, buf, found); // Recursively call the function for the subdirectory
            if (*found)                                         // If the file was found in the subdirectory, exit the loop
            {
                break;
            }
        }
    }
    closedir(dir); // Close the directory stream
}

// Func for GETDIRF
void kiay_getdirf(const char *sourceDir, const char *targetTar, const char *startDate, const char *endDate)
{
    char command[MAX_CMD_LEN]; // Declare a character array to store the command
    // Build the command using the provided parameters and store it in the 'command' array
    snprintf(command, MAX_CMD_LEN, "find %s -type f -newerBt %s ! -newerBt %s -print | tar czvf %s -T - >/dev/null 2>&1",
             sourceDir, startDate, endDate, targetTar);

    // Execute the command using the system function and store the result
    int result = system(command);
    if (result == -1)
    {
        printf("Error executing command"); // Print an error message if the command execution fails
        exit(EXIT_FAILURE);                // Exit the program with a failure status
    }
}

// Func for TARGZF
int kiay_checkFileExtensionValidity(char *filename, char **validExtensions, int numValidExtensions)
{
    char *lastDotPosition = strrchr(filename, '.'); // Find the last occurrence of the dot in the filename
    if (!lastDotPosition)
    {
        return 0; // If there's no dot in the filename, return 0 (invalid extension)
    }

    char *fileExtension = lastDotPosition + 1;   // Extract the file extension from the filename
    for (int i = 0; i < numValidExtensions; i++) // Iterate through the list of valid extensions
    {
        if (strcmp(fileExtension, validExtensions[i]) == 0) // Compare the file extension with the current valid extension
        {
            return 1; // If a valid extension is found, return 1 (valid extension)
        }
    }

    return 0; // If no valid extension is found, return 0 (invalid extension)
}

// Func for
void validateAndIncludeFiles(char *currentPath, char *tarFilename, char **validExtensions, int numValidExtensions, char *completeFilesPath)
{
    DIR *currentDir = opendir(currentPath); // Open the current directory specified by 'currentPath'
    if (!currentDir)
    {
        printf("Error opening directory"); // Print an error message if opening the directory fails
        exit(EXIT_FAILURE);                // Exit the program with a failure status
    }

    struct dirent *dirEntry; // Declare a pointer to a dirent structure
    // Loop through each entry in the current directory
    while ((dirEntry = readdir(currentDir)) != NULL)
    {
        char filePath[MAX_PATH_LENGTH];                                               // Declare a character array to store the complete file path
        snprintf(filePath, sizeof(filePath), "%s/%s", currentPath, dirEntry->d_name); // Create the complete path to the entry

        if (dirEntry->d_type == DT_DIR) // Check if the entry is a directory
        {
            if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0)
            {
                continue; // Skip current and parent directories
            }
            // Recursively call the function for the subdirectory
            validateAndIncludeFiles(filePath, tarFilename, validExtensions, numValidExtensions, completeFilesPath);
        }
        else if (dirEntry->d_type == DT_REG) // Check if the entry is a regular file
        {
            if (kiay_checkFileExtensionValidity(dirEntry->d_name, validExtensions, numValidExtensions)) // Check if the file extension is valid
            {
                printf("File: %s\n", filePath);                                                               // Print the file path
                snprintf(completeFilesPath, sizeof(completeFilesPath), "%s %s", completeFilesPath, filePath); // Append the file path to the completeFilesPath string
            }
            else
            {
                printf("Ignoring file: %s (invalid extension)\n", filePath); // Print a message for an invalid extension
            }
        }
        else
        {
            printf("Ignoring entry: %s (not a regular file or directory)\n", filePath); // Print a message for entries that are not regular files or directories
        }
    }

    if (closedir(currentDir) != 0) // Close the current directory stream
    {
        printf("Error closing directory"); // Print an error message if closing the directory fails
        exit(EXIT_FAILURE);                // Exit the program with a failure status
    }
}

// Func for TARGZF
void kiay_targzf(const char *tarFilename, char **validExtensions, int numValidExtensions)
{
    char completeFilesPath[MAX_PATH_LENGTH]; // Declare a character array to store the complete file paths
    // Call the 'validateAndIncludeFiles' function to validate files and include their paths
    validateAndIncludeFiles(getenv("HOME"), "temp.tar.gz", validExtensions, numValidExtensions, completeFilesPath);

    if (strlen(completeFilesPath) == 0) // Check if there are no valid files found
    {
        printf("No files found.\n"); // Print a message indicating that no files were found
        exit(0);                     // Exit the program with a success status
    }
    else
    {
        char command[MAX_COMMAND_LENGTH] = ""; // Declare a character array to store the command
        // Build the command to create a compressed tar archive with the specified files
        snprintf(command, sizeof(command), "tar -czvf temp.tar.gz -P %s", completeFilesPath);

        printf("Complete file paths: %s\n", completeFilesPath); // Print the complete file paths
        printf("Command: %s\n", command);                       // Print the command that will be executed

        // Execute the command using the system function
        system(command);
    }
}

// send file to client

int kiay_transferFile(int socketFd, char *filePath)
{
    int childPid = fork(); // Create a child process using the fork system call
    if (childPid == 0)     // Check if this code is executing in the child process
    {
        int fileFd, bytesRead;
        fprintf(stderr, "Copying file to client socket %d\n", socketFd); // Print a message indicating file copying

        fileFd = open(filePath, O_RDONLY); // Open the specified file for reading
        if (fileFd == -1)                  // Check if opening the file fails
        {
            printf("Error opening file"); // Print an error message
            return -1;                    // Return -1 to indicate an error
        }

        char buffer[BUFFER_SIZE];          // Declare a character array as a buffer
        memset(buffer, 0, sizeof(buffer)); // Initialize the buffer with zeros
        // Read data from the file and send it to the socket
        while ((bytesRead = read(fileFd, buffer, BUFFER_SIZE)) > 0)
        {
            ssize_t bytesSent = send(socketFd, buffer, bytesRead, 0); // Send data to the socket
            if (bytesSent == -1)                                      // Check if sending data fails
            {
                printf("Error sending data"); // Print an error message
                close(fileFd);                // Close the file descriptor
                return -1;                    // Return -1 to indicate an error
            }
            else if (bytesSent < bytesRead)
            {
                fprintf(stderr, "Partial data sent\n"); // Print a message if partial data is sent
            }
        }

        memset(buffer, 0, sizeof(buffer));             // Clear the buffer
        sleep(1);                                      // Sleep for 1 second
        fprintf(stderr, "File transfer completed \n"); // Print a success message
        close(fileFd);                                 // Close the file descriptor
        close(socketFd);                               // Close the socket descriptor
        exit(0);                                       // Exit the child process
    }

    int status;
    waitpid(childPid, &status, 0); // Wait for the child process to finish
    if (WIFEXITED(status))         // Check if the child process terminated normally
    {
        return 1; // Child process completed successfully
    }
    else
    {
        fprintf(stderr, "Child process terminated. \n"); // Print a message if the child process terminated abnormally
        return -1;                                       // Return -1 to indicate an error
    }
}

// Func for HAndeling Args
void kiay_clientArgHandleFun(int client_socket)
{
    char buffer[BUFFER_SIZE];
    int file_fd;
    char cli_mss[3030];
    char command[1024];
    char ser_mss[1024];
    char *args[MAX_ARGS];
    int kiay_num_args;
    while (1)
    {
        // Receive data from the client indicating the command
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        // Check if the client wants to exit
        // if (strcmp(buffer, ":exit") == 0)
        // {
        //     printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        //     break;
        // }
        // Receive message from the client
        int size_read = recv(client_socket, cli_mss, 2000, 0);
        if (size_read == 0) // Check if the client has disconnected
        {
            printf("Client disconnected\n");
            break;
        }
        cli_mss[size_read] = '\0';     // Add a null terminator to the received message
        printf("Client: %s", cli_mss); // Print the client's message
        char temp[1000];               // Create a temporary copy of the client's message
        sprintf(temp, "%s", cli_mss);
        kiay_num_args = 0; // Tokenize the client's message into arguments
        args[kiay_num_args] = strtok(temp, " \n");
        int transferFile = 0;
        // Loop to tokenize the remaining arguments
        while (args[kiay_num_args] != NULL && kiay_num_args < MAX_ARGS - 1)
        {
            kiay_num_args++;
            args[kiay_num_args] = strtok(NULL, " \n");
        }
        // Initialize variables for command processing
        int cmd_flag = 0;
        char message[1000];
        // Check the command and execute corresponding actions
        if (strcmp(args[0], "filesrch") == 0)
        {
            // Handle 'filesrch' command
            printf("filesrchs command is executing\n");

            // Indicate that file transfer is not needed for this command
            transferFile = 0;

            // Extract the filename to be searched for from the arguments
            char *filename = args[1];

            // Get the home directory path using getenv()
            char *path = getenv("HOME");

            // Initialize a flag to check if the file was found
            int found = 0;

            // Call the function to search for the file in the specified directory
            kiay_filesrch(client_socket, filename, path, &found);

            // If the file was not found
            if (found == 0)
            {
                // Display an error message indicating the file couldn't be found
                printf("Couldn't find the file. Make sure you enter the right file.\n");

                // Prepare an error message to send to the client
                memset(message, 0, sizeof(message));
                sprintf(message, "Couldn't find the file. Make sure you enter the right file.\n");
                message[strlen(message)] = '\0';

                // Send the error message to the client
                if (send(client_socket, message, strlen(message), 0) < 0)
                {
                    printf("Failure in sending file search error message");
                    exit(EXIT_FAILURE);
                }

                // Clear the message buffer
                memset(message, 0, sizeof(message));
            }
        }

        else if (strcmp(args[0], "targzf") == 0)
        {
            // Handle 'targzf' command
            printf("targzf command is executing\n");

            // Allocate memory to store file extensions
            char **extensions = malloc((kiay_num_args - 1) * sizeof(char *));

            // Extract and store file extensions from the arguments
            for (int i = 1; i < kiay_num_args; i++)
            {
                extensions[i - 1] = args[i];
            }

            // Calculate the number of file extensions
            int num_extensions = kiay_num_args - 1;

            // Check if the '-u' flag is present to exclude it from extension count
            if (kiay_num_args == 9 && strcmp(args[8], "-u") == 0)
            {
                num_extensions--;
            }

            // Define the name of the target tar.gz file
            char tar_filename[] = "temp.tar.gz";

            // Call the function to create the tar.gz archive with selected extensions
            kiay_targzf(tar_filename, extensions, num_extensions);

            // Indicate that a file transfer will be performed
            transferFile = 1;
        }
        else if (strcmp(args[0], "fgets") == 0)
        {
            // Check if the 'fgets' command is being executed
            printf("fgets command is executing\n");

            // Check the number of provided arguments for the command
            if (kiay_num_args < 2 || kiay_num_args > 6)
            {
                // Print an error message for an invalid number of arguments
                printf("Invalid number of arguments. Usage: fgets1 <file1> <file2> ... <file4> <-u>\n");
            }
            else
            {
                // Allocate memory to store file names
                char **files = malloc((kiay_num_args - 1) * sizeof(char *));

                // Loop through the provided arguments to extract file names
                for (int i = 1; i < kiay_num_args; i++)
                {
                    files[i - 1] = args[i]; // Store each file name in the 'files' array
                }

                // Calculate the number of files to be processed
                int num_files = kiay_num_args - 1;

                // Check if the '-u' flag is present to exclude it from the count
                if (kiay_num_args == 6 && strcmp(args[5], "-u") == 0)
                {
                    num_files--;
                }

                // Check if the number of files exceeds the allowed limit
                if (num_files > 4)
                {
                    printf("Error: Maximum of 4 files allowed.\n"); // Print an error message
                    free(files);                                    // Free the allocated memory for file names
                }
                else
                {
                    char tar_filename[] = "temp.tar.gz"; // Define the target tar.gz file name
                    int found = 0;                       // Initialize a flag for file existence

                    transferFile = 1; // Indicate that a file transfer will be performed
                }
            }
        }

        // Check the command and execute corresponding actions
        else if (strcmp(args[0], "tarfgetz") == 0)
        {
            printf("tarfgetz command is executing\n");
            transferFile = 1; // Indicate that a file transfer will be performed
            // Convert the provided arguments to long long integers
            long long size1 = atol(args[1]);
            long long size2 = atol(args[2]);
            // Check if the size parameters are valid
            if (size1 < 0 || size2 < 0 || size1 > size2)
            {
                printf("Size paramenters are not valid\n");
                return 0;
            }
            // Get the home directory path
            char home_dir[1024];
            snprintf(home_dir, sizeof(home_dir), "%s", getenv("HOME"));
            // Initialize character arrays to hold string representations of sizes
            char size1_str[20];
            char size2_str[20];
            // int argCount = sscanf(buffer + 8, "%lld %lld -u", &size1, &size2);
            // if (argCount == 3)
            // {
            //     unzip = 1;
            // }
            // else if (argCount != 2)
            // {
            //     send(newSocket, "Invalid command format", strlen("Invalid command format"), 0);
            //     continue;
            // }
            // sendMatchingFilesBySize(newSocket, size1, size2, unzip);
            // bzero(buffer, sizeof(buffer));
            // Convert sizes to string format for printing
            snprintf(size1_str, sizeof(size1_str), "%ld", size1);
            snprintf(size2_str, sizeof(size2_str), "%ld", size2);
            printf("Home Directory: %s\n", home_dir);
            printf("Size Range: %s - %s\n", size1_str, size2_str);
            kiay_tarfgetz(home_dir, "temp.tar.gz", size1_str, size2_str);
            transferFile = 1;
        }
        // Check the command and execute corresponding actions
        else if (strcmp(args[0], "getdirf") == 0)
        {
            char startDate[BUFFER_SIZE];
            char endDate[BUFFER_SIZE];
            int unzip = 0;
            struct tm ki_date1_tm, ki_date2_tm;
            printf("getdirf command is executing\n");
            memset(&ki_date1_tm, 0, sizeof(ki_date1_tm));
            memset(&ki_date2_tm, 0, sizeof(ki_date2_tm));
            if (strptime(args[1], "%Y-%m-%d", &ki_date1_tm) == NULL || strptime(args[2], "%Y-%m-%d", &ki_date2_tm) == NULL)
            {
                printf("Date formate is invalid. Please use YYYY-MM-DD\n");
            }
            else
            {
                char home_dir[1024]; // Get the home directory path
                snprintf(home_dir, 1024, "%s", getenv("HOME"));
                time_t date1 = mktime(&ki_date1_tm); // Convert parsed date structures to time_t
                time_t date2 = mktime(&ki_date2_tm);
                char f_date[20];
                char f_date2[20];
                // int argCount = sscanf(buffer + 8, "%s %s -u", startDate, endDate);
                // if (argCount == 3)
                // {
                //     unzip = 1;
                // }
                // else if (argCount != 2)
                // {
                //     send(newSocket, "Invalid command format", strlen("Invalid command format"), 0);
                //     continue;
                // }
                // Formating time_t to string according to the specified format
                strftime(f_date, sizeof(f_date), "%Y-%m-%d", &ki_date1_tm);
                strftime(f_date2, sizeof(f_date2), "%Y-%m-%d", &ki_date2_tm);
                printf("Parsed date1: %s\n", f_date);
                printf("Parsed date2: %s\n", f_date2);
                // Call the 'kiay_getdirf' function with provided parameters
                kiay_getdirf(home_dir, "temp.tar.gz", f_date, f_date2);
                transferFile = 1; // Indicate that a file transfer will be performed
            }
        }
        // Check the command and execute corresponding actions

        else if (strcmp(args[0], "quit") == 0)
        {
            printf("[-] Client exited from connection\n");
            break;
        }
        // printf("Debug: kiay_num_args = %d\n", kiay_num_args);
        // for (int i = 0; i < kiay_num_args; i++)
        // {
        //     printf("Debug: args[%d] = %s\n", i, args[i]);
        // }
        if (transferFile)
        {
            printf("Transfering file....\n");
            kiay_transferFile(client_socket, "temp.tar.gz");
        }
    }
    close(client_socket);
    // fclose(fp);
}

int main()
{
    int serverSocket, clientSocket; // Variables for Server and Client Descriptor
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen;
    int clientsHandled = 0;
    int opt = 1;

    // Creating a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        // Print if Socket creation fails
        printf("[-] Error in connection.\n");
        exit(1); // Abnormal Exit
    }

    // Configure server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;                   // Set address to I[Pv4
    serverAddr.sin_port = htons(SERVER_PORT);          // Set Port No.
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP); // Set the IP address using provided SERVER_IP

    // Bind the server address to the Socket
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printf("[-] Error! Failure in Binding!\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", SERVER_PORT);

    // Listen for incoming connections
    if (listen(serverSocket, 10) == -1)
    {
        printf("[-] Error! Failure in Binding!\n"); // Print Error of Listening fails
        exit(1);
    }

    printf("[+]Listening....\n");

    while (1)
    {
        // Accept incoming connections
        clientAddrLen = sizeof(clientAddr);
        // Accept incoming Connec. and creat new Socket
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == -1)
        {
            // Print if Accepting Connection Fails
            printf("[-] Error in accepting connection.\n");
            continue;
        }

        // Print if Client Connection successful, along with their IP Addr & port
        printf("\n[+] Client connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // Handle the first 6 client connections in the server
        if (clientsHandled < 6 || (clientsHandled >= 12 && clientsHandled % 2 == 1))
        {
            pid_t pid = fork();

            if (pid == 0)
            {
                // Child process
                printf("Client %d handled by the server\n", clientsHandled + 1);

                close(serverSocket); // Close unused server socket
                // Call to func for Client Requests
                kiay_clientArgHandleFun(clientSocket);

                exit(EXIT_SUCCESS); // Exit Child Process
            }
            else
            {
                // Increment count of handled clients and print then updated count
                clientsHandled++;
                printf("No. of Clients handled: %d\n", clientsHandled);

                // Parent process
                close(clientSocket); // Close unused client socket
                printf("Server closed client connection from %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            }
        }
        else
        {
            // Handling other cases (e.g., mirror)
            printf("[+] Mirror will handle the client", clientsHandled + 1); // Print a message indicating that the mirror will handle the client
            close(clientSocket);                                             // Close unused client socket
        }
    }

    return 0;
}
