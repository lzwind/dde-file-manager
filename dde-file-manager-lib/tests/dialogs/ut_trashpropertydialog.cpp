#include "dialogs/trashpropertydialog.h"

#include <gtest/gtest.h>

namespace  {
    class TestTrashPropertyDialog : public testing::Test
    {
    public:
        void SetUp() override
        {
            m_pTester = new TrashPropertyDialog(DUrl::fromTrashFile("/"));
            std::cout << "start TestTrashPropertyDialog";
        }
        void TearDown() override
        {
            delete m_pTester;
            m_pTester = nullptr;
            std::cout << "end TestTrashPropertyDialog";
        }
    public:
        TrashPropertyDialog *m_pTester;
    };
}

TEST_F(TestTrashPropertyDialog, testInit)
{

}

TEST_F(TestTrashPropertyDialog, testUpdateFolderSize)
{
    m_pTester->updateFolderSize(1);
}
