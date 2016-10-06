#!/bin/sh
echo "Running build..."
sh build.sh
if [[ $? = 0 ]]; then
    echo "SUCCESS! Pushing to github!"
    echo "Please enter your commit message : "
    read msg
    git add --all
    git commit -am "$msg"
    git push
else
    echo "Sorry, your build has failed... Push aborted";
    exit;
fi
