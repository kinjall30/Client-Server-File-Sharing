// Section: 3
// ID: 110100419 - Ayush Rana
// ID: 110095279 - Kinjal Prajapati
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define SERVER_IP "127.0.0.1" // IP address of the server
#define SERVER_PORT 4444      // Port no. of the server
#define BUFFER_SIZE 10000     // Buffer for sending data to server
#define MAX_PATH_LENGTH 5000
#define MAX_ARGS 7

// Handle file Fetching i.e. FGETs
void kiAyFetchfile(int socketFD, int unZipp)
{
    // Forking Child process
    int childPID = fork();

    // Child process
    if (childPID == 0)
    {
        char buff[BUFFER_SIZE];
        char tarName[1024];

        // Generating temporary File Name
        sprintf(tarName, "temp.tar.gz");

        // Opening file for writing
        int fileFD = open(tarName, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if (fileFD < 0)
        {
            printf("open");
            exit(1);
        }
        // Receiving file contents from socket
        ssize_t bytesFetched = 1;
        long int totalBytesFetched = 0;
        while (bytesFetched > 0)
        {
            memset(buff, 0, sizeof(buff));
            bytesFetched = recv(socketFD, buff, BUFFER_SIZE, 0);
            if (bytesFetched < 0)
            {
                printf("Error while receiving!");
                exit(1);
            }

            // Writing received data to the file
            long int bytesWrote = write(fileFD, buff, bytesFetched);

            if (bytesWrote < 0)
            {
                printf("Error while Writing!");
                exit(1);
            }
            // Clear buffer for next read
            memset(buff, 0, sizeof(buff));
            // Updating thhe total no. of bytes fetched
            totalBytesFetched += bytesWrote;
        }

        // Check for fetched/received data
        if (totalBytesFetched == 0)
        {
            printf("No files found.\n");
        }
        else
        {
            printf("Successfully Fetched File\n");
            printf("Total Bytes Fetched %ld\n", totalBytesFetched);
        }
        // Exiting child process
        close(fileFD);
        exit(0);
    }
    else
    {
        // Wait for Child to finish
        sleep(10);
        // Killing Child process (as it may have been finished)
        kill(childPID, SIGKILL);
        printf("Successfully Received to Parent\n");

        // Unzipp
        if (unZipp == 1)
        {
            // add
        }
    }
}

int main()
{
    int clientSocket;
    struct sockaddr_in server_addr; // Struct to store server address
    char buff[BUFFER_SIZE];
    ssize_t bytesFetched;
    ssize_t readBytes;
    int fileFD;

    // Creating socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        printf("Socket Creation Failed!");
        exit(1);
    }

    // Clear Server address
    memset(&server_addr, 0, sizeof(server_addr));

    // Setting server address to IPv4
    server_addr.sin_family = AF_INET;

    // Setting server port no.
    server_addr.sin_port = htons(SERVER_PORT);
    // Check for successful Setting of Port no. else print necessary statment
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) == -1)
    {
        printf("Error while setting Port No.");
        exit(1);
    }

    // Connection to the server
    if (connect(clientSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("Failure in Connection to Server"); // Print in case of failure
        exit(1);                                   // Abnormal Exit
    }

    // Print if connection is successful
    printf("\nConnected to Server: %s:%d\n", SERVER_IP, SERVER_PORT);

    char command[1024], serverResponse[2024];
    char *args[MAX_ARGS];
    int num_args;

    // Infinite Loop of Client
    while (1)
    {
        // Input from Command
        printf("Enter any command OR 'quit' to exit:  ");
        fgets(command, 1024, stdin);
        char temp[1024];
        sprintf(temp, "%s", command);

        num_args = 0;
        // Tokenize / Break raw command
        args[num_args] = strtok(temp, " \n");
        // Continue same procedure
        while (args[num_args] != NULL && num_args < MAX_ARGS - 1)
        {
            num_args++;
            args[num_args] = strtok(NULL, " \n");
        }

        int command_validity = 0; // Flag for validity of command
        int fileF = 0;            // File flag
        int unzip = 0;            // Flag for unzip option

        // if command is "FGETS"
        if (strcmp(args[0], "fgets") == 0)
        {
            printf(" Call for fgets...\n");
            // Check for no. of args
            if (num_args < 2 || num_args > 8 || (num_args == 2 && strcmp(args[1], "-u") == 0))
            {
                printf("Hint - 'fgets file1 [file2 ... file6] <-u>'\n"); // Print in case of improper args
            }
            else
            {
                // Set flag for file operation
                fileF = 1;
                command_validity = 1;
                // Check of '-u' option
                if (strcmp(args[num_args - 1], "-u") == 0)
                {
                    unzip = 1;
                }
            }
        }

        // if command is "TARFGETZ"
        else if (strcmp(args[0], "tarfgetz") == 0)
        {
            printf(" Call for tarfgetz...\n");
            command_validity = 1;

            // Check for no. of args
            if (num_args < 3 || num_args > 4)
            {
                printf("Hint - %s size1 size2 <-u>\n", args[0]);
                command_validity = 0;
            }
            else
            {
                // Converting arg string to long int
                long size1 = atol(args[1]);
                long size2 = atol(args[2]);

                // Check if oparameters are valid
                if (size1 < 0 || size2 < 0 || size1 > size2)
                {
                    printf("Parameter Size is Invlid!\n");
                    command_validity = 0;
                }
            }
            // If Command i.s valid
            if (command_validity)
            {
                fileF = 1;
                // Check of '-u' option
                if (strcmp(args[num_args - 1], "-u") == 0)
                {
                    unzip = 1;
                }
            }
        }

        // if command is "FILESRCH"
        else if (strcmp(args[0], "filesrch") == 0)
        {
            fileF = 0;
            printf("Call for filesrch...\n");
            // Check for no. of args
            if (num_args != 2)
            {
                fprintf(stderr, "Hint - 'filesrch filename'\n");
            }
            else
            {
                command_validity = 1;
            }
        }

        // if command is "TARGZF"
        else if (strcmp(args[0], "targzf") == 0)
        {
            printf(" Call for targzf...\n");
            // Chck for no. of args
            if (num_args < 2 || num_args > 9 || (num_args == 2 && strcmp(args[1], "-u") == 0))
            {
                printf("Hint - 'targzf ext1 ext2 ext3 ext4 [-u]'\n");
            }
            else
            {
                command_validity = 1;
                fileF = 1;
                if (strcmp(args[num_args - 1], "-u") == 0)
                {
                    unzip = 1;
                }
            }
        }

        // if command is "GETDIRF"
        else if (strcmp(args[0], "getdirf") == 0)
        {
            printf(" Call for getdirf...\n");
            command_validity = 1;

            // Check for no. of args
            if (num_args < 3 || num_args > 4)
            {
                printf("Hint - 'getdirf date1 date2 <-u>'\n");
                command_validity = 0;
            }
            else
            {
                // Initializing date structs to 0 store parsed dates
                struct tm date1_tm, date2_tm;
                memset(&date1_tm, 0, sizeof(date1_tm));
                memset(&date2_tm, 0, sizeof(date2_tm));

                // Parse and validate the date format
                if (strptime(args[1], "%Y-%m-%d", &date1_tm) == NULL || strptime(args[2], "%Y-%m-%d", &date2_tm) == NULL)
                {
                    printf("Invalid Date Format; Try: YYYY-MM-DD\n");
                    command_validity = 0;
                }
                time_t date1 = mktime(&date1_tm);
                time_t date2 = mktime(&date2_tm);

                // Check if parameters are valid
                if (date1 == -1 || date2 == -1 || date1 > date2)
                {
                    printf("Invalid Date Parameters\n");
                    command_validity = 0;
                }
            }
            if (command_validity)
            {
                fileF = 1;

                // Check for -u option
                if (strcmp(args[num_args - 1], "-u") == 0)
                {
                    unzip = 1;
                }
            }
        }

        else if (strcmp(args[0], "quit") == 0)
        {

            printf("Exit called!\n");

            break;
        }
        else
        {
            printf("Unsupported command to the server\n");
        }

        if (command_validity)
        {
            // Send command to server
            write(clientSocket, command, strlen(command));

            // Check if file fetchingg required
            if (fileF)
            {
                printf("Fetching the file...Please wait\n");
                kiAyFetchfile(clientSocket, unzip); // Call to func
            }
            else
            {
                // Receive message from server
                memset(serverResponse, 0, sizeof(serverResponse)); // Clear servr from Response buff
                // Receive data froim server
                if (recv(clientSocket, serverResponse, 2024, 0) < 0)
                {
                    printf("Error in Receiving");
                    break;
                }
                printf("Server Response: %s\n", serverResponse);
                memset(serverResponse, 0, sizeof(serverResponse)); // Clear server buff again
            }
        }
    }
    close(clientSocket);
    return 0;
}
