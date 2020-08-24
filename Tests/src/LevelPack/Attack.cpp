#include <gtest/gtest.h>
#include <LevelPack/Attack.h>
#include <LevelPack/EditorMovablePoint.h>

TEST(EditorAttackTest, Constructor) {
    EditorAttack attack(3);
    EXPECT_EQ(attack.getID(), 3);
    EXPECT_STREQ(attack.getName().c_str(), std::string().c_str());
    EXPECT_TRUE(attack.getPlayAttackAnimation());
    EXPECT_TRUE(attack.getBulletModelsCount()->empty());
}

TEST(EditorAttackTest, CopyConstructor) {
    std::shared_ptr<EditorAttack> attack = std::make_shared<EditorAttack>(42);
    attack->setName("|s0me&nam3@!@$&#)(%#!)|ASD");
    EditorAttack copy(attack);
    EXPECT_EQ(*attack, copy);
}

TEST(EditorAttackTest, Setters) {
    EditorAttack attack(3);

    attack.setID(5);
    EXPECT_EQ(attack.getID(), 5);

    attack.setName("new name");
    EXPECT_STREQ(attack.getName().c_str(), std::string("new name").c_str());

    attack.setPlayAttackAnimation(false);
    EXPECT_FALSE(attack.getPlayAttackAnimation());
}

TEST(EditorAttackTest, HitboxSearch) {
    EditorAttack attack(0);
    attack.getMainEMP()->setHitboxRadius("40");
    attack.getMainEMP()->createChild()->setHitboxRadius("75.2");
    attack.getMainEMP()->compileExpressions({});
    EXPECT_EQ(attack.searchLargestHitbox(), 75.2f);
}

TEST(EditorAttackTest, EMPSearch) {
    EditorAttack attack(0);
    std::shared_ptr<EditorMovablePoint> newEMP = attack.getMainEMP()->createChild()->createChild();
    EXPECT_EQ(attack.searchEMP(newEMP->getID()), newEMP);
}

TEST(EditorAttackTest, BulletModelsCount) {
    EditorAttack attack(0);

    std::shared_ptr<BulletModel> b1 = std::make_shared<BulletModel>(1);
    std::shared_ptr<BulletModel> b2 = std::make_shared<BulletModel>(2);
    std::shared_ptr<BulletModel> b3 = std::make_shared<BulletModel>(3);

    attack.getMainEMP()->setBulletModel(b1);
    EXPECT_EQ(attack.getBulletModelsCount()->size(), 1);
    EXPECT_EQ(attack.getBulletModelsCount()->at(1), 1);
    attack.getMainEMP()->setBulletModel(b2);
    EXPECT_EQ(attack.getBulletModelsCount()->size(), 1);
    EXPECT_EQ(attack.getBulletModelsCount()->at(2), 1);
    attack.getMainEMP()->createChild()->setBulletModel(b3);
    EXPECT_EQ(attack.getBulletModelsCount()->size(), 2);
    EXPECT_EQ(attack.getBulletModelsCount()->at(2), 1);
    EXPECT_EQ(attack.getBulletModelsCount()->at(3), 1);

    attack.load(attack.format());
    EXPECT_EQ(attack.getBulletModelsCount()->size(), 2);
    EXPECT_EQ(attack.getBulletModelsCount()->at(2), 1);
    EXPECT_EQ(attack.getBulletModelsCount()->at(3), 1);

    EditorAttack empty(1);
    attack.load(empty.format());
    EXPECT_EQ(attack.getBulletModelsCount()->size(), 0);
}