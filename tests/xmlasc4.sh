:
# Illegal UTF-8 sequence

echo -e "\0300\0274" | ./xml2asc >/dev/null
[[ $? != 0 ]]
