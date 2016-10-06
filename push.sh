#!/bin/sh
echo "Running build..."
sh build.sh
if [[ $? = 0 ]]; then
    echo "SUCCESS! Pushing to github!"
    echo "Please enter your commit message : "
    read msg
    git add --all
    MY_DATE=`date +%H:%M:%S`
    git commit -am "$msg (cert : $MY_DATE)"
    git push
else
    echo "Sorry, your build has failed... Push aborted";
    exit;
fi
