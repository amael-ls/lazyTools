#!/bin/sh

if [ $# -eq 1 ]
then
	echo "number of files" `ls -l "$1" | grep ^- | wc -l`
	echo "number of folders" `ls -l "$1" | grep ^d | wc -l`
	echo "number of links" `ls -l "$1" | grep ^l | wc -l`
else
	echo "number of files" `ls -l | grep ^- | wc -l`
	echo "number of folders" `ls -l | grep ^d | wc -l`
	echo "number of links" `ls -l | grep ^l | wc -l`
fi
