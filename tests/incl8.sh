:
trap 'rm -r $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp -d /tmp/tmp.XXXXXXXXXX` || exit 1

echo 'Test1' >$TMP3/%v%
echo 'Test2' >$TMP3/%v

(echo '<!-- include %sub%/%%v%% -->'
 echo '<!-- include %sub%/%%v -->'
 echo '<!-- include "%sub%/%%v" -->'
 echo '<!-- include %sub%/%v -->'
) | ./hxincl -s sub=%v% -s v=$TMP3 >$TMP1

(echo '<!--begin-include %sub%/%%v%% -->Test1'
 echo '<!--end-include-->'
 echo '<!--begin-include %sub%/%%v -->Test2'
 echo '<!--end-include-->'
 echo '<!--begin-include "%sub%/%%v" -->Test2'
 echo '<!--end-include-->'
 echo '<!--begin-include %sub%/%v -->Test2'
 echo '<!--end-include-->'
) >$TMP2

cmp -s $TMP1 $TMP2
