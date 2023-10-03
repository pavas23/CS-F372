# bash program to print all files in pwd
for dir in "$(pwd -P)"/*; do
    echo "$dir"
    done
