:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxref >$TMP1 <<-EOF
	<p><dfn>term</dfn> is referenced twice:
	<em title="term">here</em> and <span class="copy-this">term.</span>
EOF

# Add a newline at the end:
echo >>$TMP1

cat >$TMP2 <<-EOF
	<html><body><p><dfn id="term">term</dfn> is referenced twice:
	<a href="#term"><em title="term">here</em></a> and <a href="#term" class="copy-this">term.</a>
	</p></body></html>
EOF

cmp -s $TMP1 $TMP2
