#\!
unset glob
#set verbose
#set echo

if ( -e /mnt/tb/home/dshuman/work/usf/src/clean ) then
  echo "update clean on tb"
rsync -v -r -u -o -t -i -a --exclude="*.o"  --exclude=".deps" --exclude="*.chan" --exclude="*.log" --exclude="*.daq" --exclude="*.out" --exclude="*.gz" --exclude="*.tar" --exclude="*.cproject" --exclude=".project" --exclude="autom4te*" --exclude=".settings/" --exclude="*.cmd" --exclude="recordings/" --links --safe-links -K /home/dale/work/usf/src/clean/ /mnt/tb/home/dshuman/work/usf/src/clean
else
  echo "tb not mounted"
endif

set glob
unset verbose
unset echo




