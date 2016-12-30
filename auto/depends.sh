#!/usr/bin/bash

set -x

OBJS_DIR=objs
CONFIGURE_TOOL="./config"
OPENSSL_HOTFIX="-DOPENSSL_NO_HEARTBEATS"

	if [ ! -d ${OBJS_DIR} ];then
		`mkdir ${OBJS_DIR}`;
	fi

	if [[ -f ${OBJS_DIR}/libco/lib/libcolib.a && -d ${OBJS_DIR}/libco/include ]];then
		echo "libcolib.a  is ok";
	else
		echo "build libco begin:"
		(
			rm -rvf ${OBJS_DIR}/libco && cd ${OBJS_DIR} &&
			unzip -q ../3rdparty/libco.zip && cd libco &&
			mkdir -p include && cp *.h include &&
			make && cd ../..
		)	
	fi

        # cross build not specified, if exists flag, need to rebuild for no-arm platform.
        if [[ ! -f ${OBJS_DIR}/_flag.st.hp.tmp && -f ${OBJS_DIR}/hp/http_parser.h && -f ${OBJS_DIR}/hp/libhttp_parser.a ]]; then
            echo "http-parser-2.1 is ok.";
        else
            echo "build http-parser-2.1";
            (
                rm -rf ${OBJS_DIR}/http-parser-2.1 && cd ${OBJS_DIR} && unzip -q ../3rdparty/http-parser-2.1.zip &&
                cd http-parser-2.1 &&
                patch -p0 < ../../3rdparty/patches/2.http.parser.patch &&
                make package &&
                cd .. && rm -rf hp && ln -sf http-parser-2.1 hp &&
                cd .. && rm -f ${OBJS_DIR}/_flag.st.hp.tmp
            )
        fi

	if [[ ! -f ${OBJS_DIR}/_flag.ssl.cross.build.tmp && -f ${OBJS_DIR}/openssl/lib/libssl.a ]]; then
                echo "openssl-1.0.1f is ok.";
            else
                echo "build openssl-1.0.1f";
                (
                    rm -rf ${OBJS_DIR}/openssl-1.0.1f && cd ${OBJS_DIR} &&
                    unzip -q ../3rdparty/openssl-1.0.1f.zip && cd openssl-1.0.1f &&
                    $CONFIGURE_TOOL --prefix=`pwd`/_release -no-shared $OPENSSL_HOTFIX &&
                    make && make install_sw &&
                    cd .. && rm -rf openssl && ln -sf openssl-1.0.1f/_release openssl &&
                    cd .. && rm -f ${OBJS_DIR}/_flag.ssl.cross.build.tmp
                )
	fi
	# build  conroutine
	if [[ ! -f ${OBJS_DIR}/_flag.coroutine.tmp && -f ${OBJS_DIR}/coroutine/coroutine.h && -f ${OBJS_DIR}/coroutinelib.a ]];then
		echo "coroutine is ok";
	else
		echo "build coroutine"
		(
			rm -rf ${OBJS_DIR}/coroutine && cd ${OBJS_DIR} &&
			unzip -q ../3rdparty/coroutine.zip && cd coroutine &&
			mkdir -p include && cp *.h include &&
			make && mkdir -p lib && cp *.a lib &&
			cd ../.. && rm -f ${OBJS_DIR}/_flag.coroutine.tmp
		)
	fi

	#build jsoncpp
	if [[ ! -f ${OBJS_DIR}/_flag.jsoncpp.tmp && -f ${OBJS_DIR}/jsoncpp/include/json/json.h && -f ${OBJS_DIR}/jsoncpp/build/src/lib_json/libjsoncpp.a ]];then
		echo "jsoncpp is ok";
	else
		echo "build jsoncpp"
		(
			rm -rf ${OBJS_DIR}/jsoncpp  && cd ${OBJS_DIR} &&
			unzip -q ../3rdparty/jsoncpp.zip && cd jsoncpp &&
			mkdir -p  build && cd build && cmake ../ &&
			make -j4  && cd ../  && mkdir -p lib && cp build/src/lib_json/libjsoncpp.a lib/ &&
			cd ../.. && rm -f ${OBJS_DIR}/_flag.jsoncpp.tmp
		)
	fi
	








