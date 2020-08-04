:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/addidXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/addidXXXXXXXX` || exit 1

./hxaddid p >$TMP1 <<-EOF
	<p>This is a paragraph in English.
	<p>Voilà un écrit en français.
	<p>АБВГҐ ДЂЃЕЁ ЄЖЗЗ́Ѕ ИІЇЙЈ КЛЉМФ ХЦЧЏШ ЩЪЫЬЭ ЮЯ
	<p>αβγδε ζηθικ λμνοπ ρςστ υφχψω
EOF
echo >>$TMP1			# Add newline
cat >$TMP2 <<-EOF
	<html><body><p id="this-is-a-paragraph-in-english.-">This is a paragraph in English.
	</p><p id="voil-un-crit-en-franais.-">Voilà un écrit en français.
	</p><p id="x">АБВГҐ ДЂЃЕЁ ЄЖЗЗ́Ѕ ИІЇЙЈ КЛЉМФ ХЦЧЏШ ЩЪЫЬЭ ЮЯ
	</p><p id="x0">αβγδε ζηθικ λμνοπ ρςστ υφχψω
	</p></body></html>
EOF

cmp -s $TMP1 $TMP2
