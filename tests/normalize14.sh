:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

cat >$TMP1 <<EOF
<!DOCTYPE html>
<html lang=ja-JP>

<p>PやH2内の各行をその
  マージンの間に中央寄せして描画します。
  すると、このようになります:

<p lang=en>Here there
  are spaces.
EOF
cat >$TMP2 <<-EOF
<!DOCTYPE html>

<html lang=ja-JP>
  <body>
    <p>PやH2内の各行をそのマージンの間に中央寄せして描画します。すると、このようになります:

    <p lang=en>Here there are spaces.
EOF
./hxnormalize -i 2 $TMP1 >$TMP3

diff -u $TMP2 $TMP3
