#!/usr/bin/env bash
export CONTENT_LENGTH=0
export CONTENT_TYPE=
export GATEWAY_INTERFACE=CGI/1.1
export PATH_INFO=/
export REQUEST_URI=/
export PATH_TRANSLATED=./www/42-cgi_tester/YoupiBanane
export QUERY_STRING=
export REMOTE_ADDR=127.0.0.1
export REMOTE_HOST=127.0.0.1:46832:fd=5
export REQUEST_METHOD=GET
export SCRIPT_FILENAME=./www/42-cgi_tester/YoupiBanane/youpi.bla
export SCRIPT_NAME=youpi.bla
export SERVER_NAME=localhost
export SERVER_PORT=1234
export SERVER_PROTOCOL=HTTP/1.1
export SERVER_SOFTWARE="mofrim's WebServ"
export PATH=/nix/store/6xi5jqgw6mqdvzk36285ywc17wnx1548-nginx-1.30.1/bin:/nix/store/fr1xq5rfkv1z7mijbx1hlqb2bdqq3s36-clang-tools-22.1.5/bin:/nix/store/pdmwvbf5aimvs5ji8wgkgiv88c33k6xq-clang-wrapper-22.1.5/bin:/nix/store/qyni6c4l0dz64gz8wmv4p8673q04pfcl-clang-22.1.5/bin:/nix/store/pf30k3mg7n6bibc1k6609gyq7glk00k2-glibc-2.42-61-bin/bin:/nix/store/jjxngswsb214vb58qx485jhmilf0kxxy-coreutils-9.10/bin:/nix/store/vr4sjc5ajni6j76wqkvkx84q141270ak-binutils-wrapper-2.46/bin:/nix/store/p2vkw5s89ff1fs2d2rxqxiqil9s0jpcm-binutils-2.46/bin:/nix/store/0r6k8xa2kgqyp3r4v2w7yrb80ma2iawm-python3-3.13.12/bin:/nix/store/66lksljlljdd5ppgvfk8g89y8xgqcxd7-patchelf-0.15.2/bin:/nix/store/qd70v8g0561vm8m33kmnp79z00cgyi5n-gcc-wrapper-15.2.0/bin:/nix/store/sanx9fg8mry8mq92zhlm5qvb83qlxrlx-gcc-15.2.0/bin:/nix/store/kfwagnh6i1mysf7vxq679rzh30z9zj3g-binutils-wrapper-2.46/bin:/nix/store/s3q59l21cdhs4xvjsc6zh7j2zhi66fw1-compiler-rt-libc-22.1.5/bin:/nix/store/jjxngswsb214vb58qx485jhmilf0kxxy-coreutils-9.10/bin:/nix/store/vhsirn9m1ifmnw5g1qczzhvqkx6lw1if-findutils-4.10.0/bin:/nix/store/hx084k7pgz4n0vgkvil9gbcnl8y6p1xf-diffutils-3.12/bin:/nix/store/af4a8i43kc2ss4rnmf0swkk2mprsw6xq-gnused-4.9/bin:/nix/store/wf7lr2hf43546jc5kwqh3dbxnpcnw1mn-gnugrep-3.12/bin:/nix/store/lakv43kv98sl6h0ba6wnyg513mcq61vl-gawk-5.4.0/bin:/nix/store/rnvb7bvp53v2dw7pcwh9xb89x5z4rjib-gnutar-1.35/bin:/nix/store/9lhr1c3l9qzv8pzp3idmii1nwvxxjys3-gzip-1.14/bin:/nix/store/zj6r42syyswkhrr174bzppj3n7xhq936-bzip2-1.0.8-bin/bin:/nix/store/yvrwcs1a45rj8142n0l2w9q9s6akamjr-gnumake-4.4.1/bin:/nix/store/i27rhb3nr65rkrwz36bchkwmav6ggsmn-bash-5.3p9/bin:/nix/store/zj7mxwji29zvj9vl70iip7gw4h6ljfam-patch-2.8/bin:/nix/store/2nm5c858fh52s6mhcffm07s3biaxys44-xz-5.8.3-bin/bin:/nix/store/iscmg3ivhx7z67dz14lrg7p77gnsa4dw-file-5.45/bin:/home/mofrim/c0de/42/CC/18-webserv/.direnv/bin:/home/mofrim/bin:/home/mofrim/apps:/home/mofrim/.config/emacs/bin:/home/mofrim/bin:/home/mofrim/apps:/home/mofrim/.config/emacs/bin:/run/wrappers/bin:/home/mofrim/.nix-profile/bin:/nix/profile/bin:/home/mofrim/.local/state/nix/profile/bin:/etc/profiles/per-user/mofrim/bin:/nix/var/nix/profiles/default/bin:/run/current-system/sw/bin:/nix/store/hnan7l0v68sahyxbazz5y358prz253d1-binutils-wrapper-2.44/bin:/nix/store/igqx4mxax0a3l8rzi3akmrzwi1lvqgw2-hyprland-qtutils-0.1.5/bin:/nix/store/1hzczc3l6axcdvkbyq5v1avk25j4nzv7-pciutils-3.14.0/bin:/nix/store/6ikkxfms757m551yq8bh714q0zpysrhq-pkgconf-wrapper-2.4.3/bin:/nix/store/ib97sb3jy787b0kssfaljh3cq1r6ncpi-kitty-0.44.0/bin:/nix/store/i4lm5hvvq2cw85kyvvvzapx4k86zg2rq-imagemagick-7.1.2-18/bin:/nix/store/48kz34miglz82rcdasawjdbjrmiry2r3-ncurses-6.5-dev/bin

echo $PWD

# nix-alien ./cgi_tester ./YoupiBanane/youpi.bla
nix-alien ./cgi_tester ./YoupiBanane/youpla.bla
