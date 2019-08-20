#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <fs/FileName.h>
#include <fs/Mp3FileName.h>

using vk_music_fs::fs::FileName;
using vk_music_fs::fs::Mp3FileName;

class FileNameT: public ::testing::Test {
public:
};

TEST_F(FileNameT, Simple){ //NOLINT
    FileName fname("Artist", "Title", ".mp3");
    EXPECT_EQ(fname.getFilename(), "Artist - Title.mp3");
}

TEST_F(FileNameT, IncreaseNumbers){ //NOLINT
    FileName fname("Artist", "Title", ".mp3");
    fname.increaseNumberSuffix();
    EXPECT_EQ(fname.getFilename(), "Artist - Title_2.mp3");
    fname.increaseNumberSuffix();
    fname.increaseNumberSuffix();
    EXPECT_EQ(fname.getFilename(), "Artist - Title_4.mp3");
}

TEST_F(FileNameT, ReplaceBrackets){ //NOLINT
    Mp3FileName fname("Artist", "Title (Album version) (Album version)", ".mp3");
    EXPECT_EQ(fname.getFilename(), "Artist - Title (Album version).mp3");
}