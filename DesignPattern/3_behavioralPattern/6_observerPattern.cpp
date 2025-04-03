#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;

// 前向声明
class Subject;

// 观察者接口
class Observer{
public:
    virtual void update(Subject* subject) = 0;
    virtual ~Observer() = default;
};

// 主题接口
class Subject{
private:
    vector<Observer*> observers_;
public:
    virtual ~Subject() = default;  
    virtual void attach(Observer* observer) {
        observers_.push_back(observer);
    }
    virtual void detach(Observer* observer){
        observers_.erase(remove(observers_.begin(),observers_.end(),observer), observers_.end());
    }
    virtual void notifyObservers() {
        for(auto observer: observers_){
            observer->update(this);
        }
    }
};

// 具体主题：气象站
class WeatherStation : public Subject {
private:
    float temperature_;
    float humidity_;
    float pressure_;
public:
    void setMeasurements(float temperature, float humidity, float pressure) {
        temperature_ = temperature;
        humidity_ = humidity;
        pressure_ = pressure;
        notifyObservers();
    }

    float getTemperature() const { return temperature_; }
    float getHumidity() const { return humidity_; }
    float getPressure() const { return pressure_; }
};


// 具体观察者：当前天气状况显示 显示当前天气数据
class CurrentConditionsDisplay : public Observer {
private:
    WeatherStation* weatherStation_;
    float temperature_;
    float humidity_;
public:
    CurrentConditionsDisplay(WeatherStation* weatherStation)
            : weatherStation_(weatherStation) {
            weatherStation_->attach(this);
    }
    void update(Subject* subject) override {
        if (auto ws = dynamic_cast<WeatherStation*>(subject)) {
            temperature_ = ws->getTemperature();
            humidity_ = ws->getHumidity();
            display();
        }
    }
    void display() const {
        cout << "Current conditions: "
                  << temperature_ << "°C and "
                  << humidity_ << "humidity\n";
    }

};

// 具体观察者：天气预报显示 根据气压变化显示天气预报
class ForecastDisplay : public Observer {
public:
    explicit ForecastDisplay(WeatherStation* weatherStation)
        : weatherStation_(weatherStation) {
        weatherStation_->attach(this);
    }

    void update(Subject* subject) override {
        if (auto ws = dynamic_cast<WeatherStation*>(subject)) {
            lastPressure_ = currentPressure_;
            currentPressure_ = ws->getPressure();
            display();
        }
    }

    void display() const {
        cout << "Forecast: ";
        if (currentPressure_ > lastPressure_) {
            cout << "Improving weather!\n";
        } else if (currentPressure_ == lastPressure_) {
            cout << "More of the same\n";
        } else {
            cout << "Watch out for cooler, rainy weather\n";
        }
    }

private:
    WeatherStation* weatherStation_;
    float currentPressure_ = 1013.0f;
    float lastPressure_ = 1013.0f;
};


int main() {
    WeatherStation weatherStation;
    
    CurrentConditionsDisplay currentDisplay(&weatherStation);
    ForecastDisplay forecastDisplay(&weatherStation);

    // 模拟天气数据更新
    weatherStation.setMeasurements(25.0f, 65.0f, 1015.0f);
    weatherStation.setMeasurements(22.5f, 70.0f, 1012.0f);
    weatherStation.setMeasurements(20.0f, 90.0f, 1008.0f);

    return 0;
}


//简单版本
// #include <iostream>
// #include <vector>

// // 观察者接口
// class Observer {
// public:
//     virtual void update(float temperature) = 0;
//     virtual ~Observer() = default;
// };

// // 主题（被观察者）
// class TemperatureSensor {
//     std::vector<Observer*> observers_;
//     float temperature_;

// public:
//     void attach(Observer* observer) {
//         observers_.push_back(observer);
//     }

//     void setTemperature(float newTemp) {
//         temperature_ = newTemp;
//         notifyObservers();
//     }

// private:
//     void notifyObservers() {
//         for (auto observer : observers_) {
//             observer->update(temperature_);
//         }
//     }
// };

// // 具体观察者
// class Display : public Observer {
// public:
//     void update(float temperature) override {
//         std::cout << "温度更新: " << temperature << "°C\n";
//     }
// };

// int main() {
//     TemperatureSensor sensor;
//     Display display;

//     sensor.attach(&display);

//     // 模拟温度变化
//     sensor.setTemperature(25.5);
//     sensor.setTemperature(23.0);
//     sensor.setTemperature(20.5);

//     return 0;
// }