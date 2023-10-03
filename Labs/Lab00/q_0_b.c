/*
Linux file permissions determine who can access files and directories on a system and how.
use ls -l (long listing) will show metadata about Linux files including permission set on a file

$ ls -l
-rw-r--r--. 1 root root  4017 Feb 24  2022 vimrc

File type: -
Permission settings: rw-r--r--
Extended attributes: dot (.)
User owner: root
Group owner: root

to modify permissions use chmod command which means (change mode).

To change file permissions in numeric mode, you enter chmod and the octal value you desire, such as 744, alongside the file name. To change file permissions in symbolic mode, you enter a user class and the permissions you want to grant them next to the file name.

$ chmod ug+rwx example.txt
$ chmod o+r example2.txt

This grants read, write, and execute for the user and group, and only read for others. In symbolic mode, chmod u represents permissions for the user owner, chmod g represents other users in the file's group, chmod o represents other users not in the file's group. For all users, use chmod a.

Maybe you want to change the user owner itself. You can do that with the chown command. Similarly, the chgrp command can be used to change the group ownership of a file.
*/

/*
diff command is used to find differences between files
*/

/*
top is the older command and it comes preinstalled on all Linux distros. htop is newer and it adds color and gives a more interactive user interface than top. In other words, htop provides a slightly better user experience, but it's not a standard command you'll find on all Linux systems.
*/

/*
cp - stands for copy
It creates an exact image of a file on a disk with a different file name.

cp [OPTION] Source Destination
cp [OPTION] Source Directory
cp [OPTION] Source-1 Source-2 Source-3 Source-n Directory

The first and second syntax is used to copy the Source file to the Destination file or Directory.
The third syntax is used to copy multiple Sources(files) to the Directory.

mv
Two Distinct Functions of `mv` Command
1) Renaming a file or directory.
2) Moving a file or directory to another location

mv [source_file_name(s)] [Destination_file_name]
mv jayesh_gfg geeksforgeeks // renames the file

mv [source_file_name(s)] [Destination_path]
mv geeksforgeeks /home/jayeshkumar/jkj/  // this will move file to the directory

// for moving multiple files in linux
mv [source_file_name_1] [source_file_name_2] [source_file_name_ .....] [Destination_path]

rm is used for removing files or directories
Removing more than one file at a time
$ rm b.txt c.txt

pwd
pwd stands for Print Working Directory. It prints the path of the working directory, starting from the root.
*/

/*
SCP stands for Secure Copy Protocol and is a way to transfer files from one computer to another securely. SCP (Secure Copy) is a command-line utility that uses SSH to securely transfer files between computers, allowing for secure authentication, encryption, and data integrity.

It is typically used to copy files from a local computer to a remote server or from remote servers to a local computer.

cp [options] source_file [user@]host1:destination_file

The source_file is the file you want to transfer, and the destination_file is the location to which you want the file to be copied. The user@ portion is optional and is used to specify the username for authentication when connecting to the remote host. However, when copying multiple files, you must specify the file name as the source path.

The SCP command works by establishing an SSH connection between two computers and then copying files from one computer to the other. It uses public-key cryptography to encrypt the data and authenticate the user. This means that the data is safe from interception while in transit and only the user with the correct key can access the data.
*/
