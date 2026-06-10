#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditLayer.hpp>
#include <cclog/cclog.hpp>

using namespace geode::prelude;

// Hook vào màn hình chỉnh sửa Level (LevelEditLayer) của Geometry Dash
class $modify(MyAILevelEditor, LevelEditLayer) {
    bool init(GJGameLevel* level) {
        if (!LevelEditLayer::init(level)) return false;

        // 1. Tạo một nút bấm "AI Edit" trên giao diện chỉnh sửa của GD
        auto menu = this->getChildByID("back-menu"); // Thêm tạm vào menu quay lại
        
        auto aiButtonSprite = ButtonSprite::create("AI", "goldFont.fnt", "GJ_button_01.png", 0.6f);
        auto aiButton = CCMenuItemSpriteExtra::create(
            aiButtonSprite,
            this,
            menu_selector(MyAILevelEditor::onAIButtonClick)
        );
        
        menu->addChild(aiButton);
        menu->updateLayout();

        return true;
    }

    void onAIButtonClick(CCObject* sender) {
        // Trong thực tế, bạn sẽ tạo một ô nhập văn bản (TextInput) trong GD để gõ ý tưởng
        std::string userPrompt = "Create a simple jump with 3 blocks and 1 spike";
        
        // 2. Gửi yêu cầu HTTP POST tới Python Server đang chạy ngầm
        web::WebRequest req = web::WebRequest();
        matjson::Value body;
        body["prompt"] = userPrompt;
        body["level_id"] = "123456"; // Giả lập truyền ID level hiện tại

        req.json(body).post("http://127.0.0").listen([this](web::WebResponse* res) {
            if (res->isSuccess()) {
                auto json = res->json().value();
                std::string gdString = json["gd_level_string"].asString().unwrap_or("");
                
                // 3. Nạp chuỗi block từ AI trực tiếp vào màn chơi đang mở
                // m_level là con trỏ chứa dữ liệu màn chơi hiện tại trong LevelEditLayer
                if (!gdString.empty() && this->m_level) {
                    std::string currentLevelData = this->m_level->m_levelString;
                    // Nối chuỗi AI tạo vào cuối dữ liệu level cũ
                    this->m_level->m_levelString = currentLevelData + gdString;
                    
                    // Thông báo cho người chơi và tải lại giao diện
                    FLAlertLayer::create("AI Editor", "AI has successfully added objects!", "OK")->show();
                }
            } else {
                FLAlertLayer::create("Error", "Failed to connect to AI Server.", "OK")->show();
            }
        });
    }
};
