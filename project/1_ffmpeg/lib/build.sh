PREFIX="/home/ubuntu2204/ffmpge/out_one"  

build() {   
    make clean
    ./configure \
        --disable-shared \
        --enable-static \
        --disable-doc \
        --disable-programs \
        --enable-avcodec \
        --enable-avfilter \
        --enable-avformat \
        --enable-avutil \
        --enable-swscale \
        --enable-swresample \
        --prefix="$PREFIX" \
        --extra-cflags="-fPIC" \
        --extra-ldflags="-Wl,-rpath='\$ORIGIN'" 

    make -j$(nproc) 
    make install

    # 使用gcc链接共享库
    gcc -shared -o libffmpeg-zzq.so \
        -Wl,--whole-archive \
        "$PREFIX/lib/libavcodec.a" \
        "$PREFIX/lib/libavfilter.a" \
        "$PREFIX/lib/libavformat.a" \
        "$PREFIX/lib/libswscale.a" \
        "$PREFIX/lib/libswresample.a" \
        "$PREFIX/lib/libavutil.a" \
        -Wl,--no-whole-archive \
        -Wl,-Bsymbolic -Wl,--no-undefined \
        -lm -lz -pthread -ldl
}

# 执行编译
build