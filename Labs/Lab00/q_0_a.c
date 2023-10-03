/* In Linux, Top command is utilized to monitor Linux Process which is used frequently used by many systems. All the running and active real-time processes in ordered list is displayed and updates it regularly by this Top command. Display CPU usage, Swap memory, Cache Size, Buffer Size,Process PID, User, Commands and much more. It shows high memory and CPU utilization of running processes in your machine. */

/* PID's or Process ID's are numbers assigned to processes to differentiate from other processes */
/* Pipes are connections between two processes which allows IPC (interprocess communication). The output of one process goes as input to another process. */

/*  get pid of a process using pgrep
    pgrep firefox // this will give pid as output for the process
    kill -15 pid
 */

/* While du reports files' and directories' disk usage, df reports how much disk space your filesystem is using. The df command displays the amount of disk space available on the filesystem with each file name's argument.
*/

/* Hard Link is a link between file name and data stored in the memory, be default each file points or has only a single hard link. We can create a new file and add a hard link pointing to data of old file. Now we have hard links pointing to same data. Changes made in any file will reflect in other file as well.

Data will be deleted only if all hard links are deleted.

[tcarrigan@server demo]$ ls -l
total 4
-rw-rw-r--. 1 tcarrigan tcarrigan 12 Aug 29 14:27 link_test

[tcarrigan@server demo]$ ln link_test /tmp/link_new

ln (original file path) (new file path) // syntax for adding a hard link for a new file

[tcarrigan@server demo]$ ls -l link_test /tmp/link_new 
-rw-rw-r--. 2 tcarrigan tcarrigan 12 Aug 29 14:27 link_test
-rw-rw-r--. 2 tcarrigan tcarrigan 12 Aug 29 14:27 /tmp/link_new

hard link count now became 2

Files that are hard linked together share the same inode number

[tcarrigan@server demo]$ ls -li link_test /tmp/link_new 
2730074 -rw-rw-r--. 2 tcarrigan tcarrigan 12 Aug 29 14:27 link_test
2730074 -rw-rw-r--. 2 tcarrigan tcarrigan 12 Aug 29 14:27 /tmp/link_new

The shared inode number is 2730074, meaning these files are identical data.
*/

/* A symbolic link, also termed a soft link, is a special kind of file that points to another file, much like a shortcut in Windows or a Macintosh alias. Unlike a hard link, a symbolic link does not contain the data in the target file. It simply points to another entry somewhere in the file system. This difference gives symbolic links certain qualities that hard links do not have, such as the ability to link to directories, or to files on remote computers networked through NFS. Also, when you delete a target file, symbolic links to that file become unusable, whereas hard links preserve the contents of the file. 
*/

/*  command to create a symbolic link
    ln -s source_file myfile
    cp src dest
    inode number of 1.c 2.c and 3.c are different, as 2.c is a soft link to 1.c so they both do not point to same data
    after deleting 1.c, we can not access 2.c, if we try to copy 2.c to 4.c it gives error as no such file exists.
*/

/*
sudo (Super User DO) command in Linux is generally used as a prefix for some commands that only superusers are allowed to run. If you prefix any command with “sudo”, it will run that command with elevated privileges or in other words allow a user with proper permissions to execute a command as another user, such as the superuser. This is the equivalent of the “run as administrator” option in Windows. The option of sudo lets us have multiple administrators.
*/

