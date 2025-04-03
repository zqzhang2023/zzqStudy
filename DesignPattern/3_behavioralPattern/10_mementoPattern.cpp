// #include <iostream>
// #include <string>
// #include <vector>
// using namespace std;

// // 白盒实现：Memento 内部状态对 Caretaker 可见
// class TextMemento{
// private:
//     string text_;
// public:
//     TextMemento(string text) : text_(move(text)) {}
//     // 暴露内部状态的公共方法
//     string GetText() const { return text_; }
//     void SetText(const string& text) { text_ = text; }
// };

// // Originator（原发器）
// class TextEditor {
// private:
//     string content_;
// public:
//     void Write(const string& text) { content_ += text; }
//     TextMemento CreateMemento() const {
//         return TextMemento(content_);
//     }
//     void RestoreMemento(const TextMemento& memento) {
//         content_ = memento.GetText();
//     }
//     void Show() const {
//         cout << "Current content: " << content_ << "\n";
//     }
// };

// // Caretaker（管理者）
// class History {
// private:
//     vector<TextMemento> history_;
// public:
//     void Save(const TextMemento& memento) {
//         history_.push_back(memento);
//     }
//     TextMemento Undo() {
//         if (!history_.empty()) {
//             return history_.back();
//         }
//         return TextMemento("");
//     }
// };

// int main(){

//     TextEditor editor;
//     History history;
    
//     editor.Write("Hello");
//     history.Save(editor.CreateMemento());
//     editor.Show();
    
//     editor.Write(" World");
//     editor.Show();
    
//     editor.RestoreMemento(history.Undo());
//     editor.Show();


//     return 0;
// }

#include <iostream>
#include <string>
#include <vector>

// 抽象备忘录接口
class IMemento {
public:
    virtual ~IMemento() = default;
};

// Originator（原发器）
class TextEditor {
private:
    std::string content_;
public:
    // 内部 Memento 实现
    class TextMemento : public IMemento {
    public:
        TextMemento(std::string text) : text_(std::move(text)) {}
        // 只有 TextEditor 可以访问私有成员
        friend class TextEditor;
    private:
        std::string text_;
    };

    void Write(const std::string& text) { content_ += text; }

    IMemento* CreateMemento() const {
        return new TextMemento(content_);
    }

    void RestoreMemento(const IMemento* memento) {
        auto concrete = dynamic_cast<const TextMemento*>(memento);
        if (concrete) {
            content_ = concrete->text_;
        }
    }

    void Show() const {
        std::cout << "Current content: " << content_ << "\n";
    }

};


// Caretaker（管理者）
class History {
private:
    std::vector<IMemento*> history_;
public:
    void Save(IMemento* memento) {
        history_.emplace_back(memento);
    }
    
    IMemento* Undo() {
        if (!history_.empty()) {
            return history_.back();
        }
        return nullptr;
    }
};

// 使用示例
int main() {
    TextEditor editor;
    History history;
    
    editor.Write("Hello");
    history.Save(editor.CreateMemento());
    editor.Show();
    
    editor.Write(" World");
    editor.Show();
    
    editor.RestoreMemento(history.Undo());
    editor.Show();
}
