#!/bin/bash
IFS_save=$IFS
IFS=$'\n' # to fix Internal Field Separator

echo "You are running $0 with arg = $1"

sed -i '' '/file = {/d' $1
sed -i '' '/abstract = {/d' $1
# sed -i '' '/doi = {/d' $1
sed -i '' '/issn = {/d' $1
sed -i '' '/keywords = {/d' $1
sed -i '' '/pmid = {/d' $1
# sed -i '' '/url = {/d' $1
sed -i '' '/isbn = {/d' $1
sed -i '' '/archivePrefix = {/d' $1
# sed -i '' '/arxivId = {/d' $1
sed -i '' '/shorttitle = {/d' $1
sed -i '' '/language = {/d' $1
sed -i '' '/eprint = {/d' $1
sed -i '' '/annote = {/d' $1
sed -i '' '/urldate = {/d' $1
sed -i '' '/local-url = {/d' $1

perl -i -pe 's/\xE2\x80\x90/-/g' $1 # For character U+2010 (UTF-8 code)
perl -i -pe 's/\xE2\x80\x93/-/g' $1 # For character U+2013 (UTF-8 code)

if [ "$#" -eq 2 ]
then
	sed -i '' 's/month = {jan}/month = {1}/' $1
	sed -i '' 's/month = {feb}/month = {2}/' $1
	sed -i '' 's/month = {mar}/month = {3}/' $1
	sed -i '' 's/month = {apr}/month = {4}/' $1
	sed -i '' 's/month = {may}/month = {5}/' $1
	sed -i '' 's/month = {jun}/month = {6}/' $1
	sed -i '' 's/month = {jul}/month = {7}/' $1
	sed -i '' 's/month = {aug}/month = {8}/' $1
	sed -i '' 's/month = {sep}/month = {9}/' $1
	sed -i '' 's/month = {oct}/month = {10}/' $1
	sed -i '' 's/month = {nov}/month = {11}/' $1
	sed -i '' 's/month = {dec}/month = {12}/' $1
fi
