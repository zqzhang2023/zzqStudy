#include<iostream>
using namespace std;

//子系统类：音频解码器
class AudioDecoder{
public:
    void decodeAudio(const string& file){
        cout << "解码音频: " << file << endl;
    }
};

//子系统类：视频解码器
class VideoDecoder {
public:
    void decodeVideo(const string& file){
        cout << "解码视频: " << file << endl;
    }
};

//子系统类：文件加载器
class FileLoader {
public:
    string load(const string& path){
        cout << "加载文件: " << path << endl;
        return path.substr(path.find_last_of("/") + 1);
    }
};

//外观类：封装多媒体播放的复杂操作
class PlaybackManager{
private:
    FileLoader    fileLoader;
    AudioDecoder  audioDecoder;
    VideoDecoder  videoDecoder;

public:
    void play(const string& filePath) {
        string fileName = fileLoader.load(filePath);
        audioDecoder.decodeAudio(fileName);
        videoDecoder.decodeVideo(fileName);
        cout << "开始播放 " << fileName << endl;
    }

};

// 客户端代码
int main() {
    PlaybackManager player;
    player.play("/media/movie.mp4");
    return 0;
}