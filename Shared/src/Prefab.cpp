#include "Prefab.h"

void AddNewPrefabNode(PrefabNode* parentNode, std::string newName) {
    PrefabNode* newNode = new PrefabNode;
    newNode->m_Name = newName;
    newNode->m_Parent = parentNode;
    parentNode->m_Children.push_back(newNode);
}
void RemovePrefabNode(PrefabNode* NodeToRemove) {
    if(!NodeToRemove->m_Parent) return;

    auto& childrenArray = NodeToRemove->m_Parent->m_Children;
    for(auto it = childrenArray.begin(); it != childrenArray.end();) {
        PrefabNode* p = *it;
        if(p == NodeToRemove) {
            delete p;
            it = childrenArray.erase(it);
        } else {
            it++;
        }
    }
}