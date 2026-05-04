#!/usr/bin/env bash

function arrContains() {
	if [ $# -le 2 ]; then
		echo "need 2 args: elem arr"
		exit 1
	fi
	local cur
	local match="$1"
	shift
	for cur; do
		[[ "$cur" == "$match" ]] && return 0
	done
	return 1
}

arr=(
	"moep"
	"miep"
	"at at"
)

if arrContains moep "${arr[@]}"; then echo "moep is inside"; fi
echo
if arrContains miep "${arr[@]}"; then echo "miep is inside"; fi
echo
if ! arrContains adjasljk "${arr[@]}"; then echo "klajsdlajf is not inside"; fi
echo
if arrContains "at at" "${arr[@]}"; then echo "'at at' is inside"; fi
echo
if ! arrContains "at" "${arr[@]}"; then echo "'at' is not inside"; fi
