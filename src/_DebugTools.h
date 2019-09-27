#include "Object.h"
#include "_Util.h"
#include "imgui-SFML.h"
#include "imgui.h"

namespace ng
{
class _DebugTools
{
  public:
    explicit _DebugTools(Engine &engine) : _engine(engine) {}

    void render()
    {
        ImGui::Begin("Debug");
        std::stringstream s;
        s << "Stack: " << sq_gettop(_engine.getVm());
        ImGui::TextUnformatted(s.str().c_str());
        ImGui::Text("In cutscene: %s", _engine.inCutscene() ? "yes" : "no");
        ImGui::Text("In dialog: %s", _engine.getDialogManager().isActive() ? "yes" : "no");
        ImGui::Separator();
        ImGui::Checkbox("Actors", &_showActors);
        ImGui::Checkbox("Objects", &_showObjects);
        ImGui::Checkbox("Rooms", &_showRooms);
        ImGui::Separator();
        auto &rooms = _engine.getRooms();
        int currentRoom = 0;
        for (int i = 0; i < rooms.size(); i++)
        {
            if (rooms[i].get() == _engine.getRoom())
            {
                currentRoom = i;
                break;
            }
        }
        if (ImGui::Combo("Room", &currentRoom, roomGetter, static_cast<void *>(&rooms), rooms.size()))
        {
            _engine.setRoom(rooms[currentRoom].get());
        }
        ImGui::End();

        if (_showActors)
        {
            showActors();
        }

        if (_showObjects)
        {
            showObjects();
        }

        if (_showRooms)
        {
            showRooms();
        }
    }

  private:
    void showActors()
    {
        static auto actor_getter = [](void *vec, int idx, const char **out_text) {
            auto &vector = *static_cast<std::vector<std::unique_ptr<Actor>> *>(vec);
            if (idx < 0 || idx >= static_cast<int>(vector.size()))
            {
                return false;
            }
            *out_text = vector.at(idx)->getName().c_str();
            return true;
        };

        ImGui::Begin("Actors", &_showActors);
        auto &actors = _engine.getActors();
        ImGui::Combo("", &_selectedActor, actor_getter, static_cast<void *>(&actors), actors.size());
        auto &actor = actors[_selectedActor];
        auto isVisible = actor->isVisible();
        if (ImGui::Checkbox("visible", &isVisible))
        {
            actor->setVisible(isVisible);
        }
        auto isTouchable = actor->isTouchable();
        if (ImGui::Checkbox("touchable", &isTouchable))
        {
            actor->setTouchable(isTouchable);
        }
        ImGui::Text("talking: %s", actor->isTalking() ? "true" : "false");
        ImGui::Text("walking: %s", actor->isWalking() ? "true" : "false");
        auto talkColor = actor->getTalkColor();
        if (ColorEdit4("talk color", talkColor))
        {
            actor->setTalkColor(talkColor);
        }
        ImGui::End();
    }

    void showObjects()
    {
        static auto objectGetter = [](void *vec, int idx, const char **out_text) {
            auto &vector = *static_cast<std::vector<std::unique_ptr<Object>> *>(vec);
            if (idx < 0 || idx >= static_cast<int>(vector.size()))
            {
                return false;
            }
            *out_text = tostring(vector.at(idx)->getId()).c_str();
            return true;
        };

        ImGui::Begin("Objects", &_showObjects);
        auto &objects = _engine.getRoom()->getObjects();
        ImGui::Combo("", &_selectedObject, objectGetter, static_cast<void *>(&objects), objects.size());
        auto &object = objects[_selectedObject];
        auto isVisible = object->isVisible();
        ImGui::TextUnformatted(tostring(object->getName()).c_str());
        if (ImGui::Checkbox("visible", &isVisible))
        {
            object->setVisible(isVisible);
        }
        auto zorder = object->getZOrder();
        if (ImGui::InputInt("zorder", &zorder))
        {
            object->setZOrder(zorder);
        }
        auto isTouchable = object->isTouchable();
        if (ImGui::Checkbox("touchable", &isTouchable))
        {
            object->setTouchable(isTouchable);
        }
        ImGui::End();
    }

    void showRooms()
    {
        ImGui::Begin("Rooms", &_showRooms);
        auto &rooms = _engine.getRooms();
        ImGui::Combo("", &_selectedRoom, roomGetter, static_cast<void *>(&rooms), rooms.size());
        auto &room = rooms[_selectedRoom];
        auto rotation = room->getRotation();
        if (ImGui::InputFloat("rotation", &rotation))
        {
            room->setRotation(rotation);
        }
        auto ambient = room->getAmbientLight();
        if (ColorEdit4("ambient", ambient))
        {
            room->setAmbientLight(ambient);
        }
        ImGui::End();
    }

    static bool roomGetter(void *vec, int idx, const char **out_text)
    {
        auto &vector = *static_cast<std::vector<std::unique_ptr<Room>> *>(vec);
        if (idx < 0 || idx >= static_cast<int>(vector.size()))
        {
            return false;
        }
        *out_text = vector.at(idx)->getId().c_str();
        return true;
    }

    static bool ColorEdit4(const char *label, sf::Color &color)
    {
        float imColor[4] = {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
        if (ImGui::ColorEdit4(label, imColor))
        {
            color.r = static_cast<sf::Uint8>(imColor[0] * 255.f);
            color.g = static_cast<sf::Uint8>(imColor[1] * 255.f);
            color.b = static_cast<sf::Uint8>(imColor[2] * 255.f);
            color.a = static_cast<sf::Uint8>(imColor[3] * 255.f);
            return true;
        }
        return false;
    }

  private:
    Engine &_engine;
    bool _showActors{true};
    bool _showObjects{true};
    bool _showRooms{false};
    int _selectedActor{0};
    int _selectedObject{0};
    int _selectedRoom{0};
};
} // namespace ng
