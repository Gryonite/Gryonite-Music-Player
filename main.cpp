#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QSlider>
#include <QStackedWidget>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QListWidget>
#include <QFileDialog>
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QListWidgetItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaMetaData>
#include <QUrl>
#include <QPixmap>
#include <QImage>
#include <QVariant>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <functional>

// FORMAT TIME
QString formatTime(qint64 milliseconds)
{
    qint64 totalSeconds = milliseconds / 1000;
    qint64 minutes = totalSeconds / 60;
    qint64 seconds = totalSeconds % 60;

    return QString("%1:%2")
        .arg(minutes)
        .arg(seconds, 2, 10, QChar('0'));
}

// CREATE PAGE
QWidget* createPage(const QString& title, const QString& description)
{
    QWidget* page = new QWidget();

    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(35, 30, 35, 20);
    layout->setSpacing(18);

    QLabel* heading = new QLabel(title);
    heading->setObjectName("heading");

    QLabel* subtitle = new QLabel(description);
    subtitle->setObjectName("subtitle");

    layout->addWidget(heading);
    layout->addWidget(subtitle);
    layout->addStretch();

    return page;
}

// MAIN
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("Gryonite Music Player");
    window.resize(1250, 800);
    window.setMinimumSize(950, 600);

    // SQLITE DATABASE
    QString appDataPath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QDir().mkpath(appDataPath);

    QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(appDataPath + "/gryonite.db");

    if (!database.open()) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            "Gryonite could not open its music library database:\n" +
            database.lastError().text()
        );

        return 1;
    }

    QSqlQuery databaseQuery;

    if (!databaseQuery.exec(
        "CREATE TABLE IF NOT EXISTS music_folders ("
        "path TEXT PRIMARY KEY)"
    )) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            databaseQuery.lastError().text()
        );
        return 1;
    }

    if (!databaseQuery.exec(
        "CREATE TABLE IF NOT EXISTS playlists ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT UNIQUE)"
    )) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            databaseQuery.lastError().text()
        );
        return 1;
    }

    // AUDIO PLAYER
    QMediaPlayer* mediaPlayer = new QMediaPlayer(&window);
    QAudioOutput* audioOutput = new QAudioOutput(&window);

    mediaPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.70);

    // MAIN WIDGET
    QWidget* central = new QWidget();
    window.setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->setSpacing(0);

    // SIDEBAR
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(230);
    sidebar->setObjectName("sidebar");

    QVBoxLayout* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(20, 25, 20, 20);
    sideLayout->setSpacing(12);

    QLabel* logo = new QLabel("GRYONITE");
    logo->setObjectName("logo");
    sideLayout->addWidget(logo);

    sideLayout->addSpacing(25);

    QPushButton* homeButton = new QPushButton("⌂   Home");
    QPushButton* searchButton = new QPushButton("⌕   Search");
    QPushButton* libraryButton = new QPushButton("▤   Your Library");

    sideLayout->addWidget(homeButton);
    sideLayout->addWidget(searchButton);
    sideLayout->addWidget(libraryButton);

    sideLayout->addSpacing(30);

    QLabel* playlistTitle = new QLabel("YOUR PLAYLISTS");
    playlistTitle->setObjectName("sectionLabel");
    sideLayout->addWidget(playlistTitle);

    QPushButton* createPlaylistButton =
        new QPushButton("+   Create Playlist");

    QPushButton* likedButton =
        new QPushButton("♡   Liked Songs");

    sideLayout->addWidget(createPlaylistButton);
    sideLayout->addWidget(likedButton);

    QVBoxLayout* playlistLayout = new QVBoxLayout();
    playlistLayout->setSpacing(5);
    sideLayout->addLayout(playlistLayout);

    sideLayout->addStretch();

    QLabel* version = new QLabel("Gryonite Music Player\nVersion 0.6");
    version->setObjectName("version");
    sideLayout->addWidget(version);

    // PAGE SYSTEM
    QStackedWidget* pages = new QStackedWidget();

    // HOME PAGE
    QWidget* homePage = new QWidget();
    QVBoxLayout* homeLayout = new QVBoxLayout(homePage);

    homeLayout->setContentsMargins(35, 30, 35, 20);
    homeLayout->setSpacing(20);

    QLabel* welcome = new QLabel("Welcome to Gryonite");
    welcome->setObjectName("heading");

    QLabel* subtitle = new QLabel(
        "Your music, your library, your way."
    );
    subtitle->setObjectName("subtitle");

    QLabel* recentTitle = new QLabel("Recently Played");
    recentTitle->setObjectName("sectionHeading");

    homeLayout->addWidget(welcome);
    homeLayout->addWidget(subtitle);
    homeLayout->addSpacing(15);
    homeLayout->addWidget(recentTitle);

    QHBoxLayout* albumLayout = new QHBoxLayout();
    albumLayout->setSpacing(18);

    for (int i = 1; i <= 4; i++) {
        QFrame* card = new QFrame();
        card->setObjectName("albumCard");
        card->setMinimumSize(145, 175);

        QVBoxLayout* cardLayout = new QVBoxLayout(card);

        QLabel* cover = new QLabel("♫");
        cover->setObjectName("albumCover");
        cover->setAlignment(Qt::AlignCenter);
        cover->setMinimumHeight(110);

        QLabel* albumName = new QLabel("Your Music");
        albumName->setObjectName("albumName");

        QLabel* artist = new QLabel("Local Library");
        artist->setObjectName("artistName");

        cardLayout->addWidget(cover);
        cardLayout->addWidget(albumName);
        cardLayout->addWidget(artist);

        albumLayout->addWidget(card);
    }

    homeLayout->addLayout(albumLayout);

    QLabel* homeLibraryHeading = new QLabel("Your Library");
    homeLibraryHeading->setObjectName("sectionHeading");

    QLabel* emptyMessage = new QLabel(
        "Your music collection will appear here.\n"
        "Import your local music to get started."
    );
    emptyMessage->setObjectName("emptyMessage");

    homeLayout->addSpacing(20);
    homeLayout->addWidget(homeLibraryHeading);
    homeLayout->addWidget(emptyMessage);
    homeLayout->addStretch();

    // SEARCH PAGE
    QWidget* searchPage = createPage(
        "Search",
        "Find music in your local collection."
    );

    QVBoxLayout* searchLayout =
        qobject_cast<QVBoxLayout*>(searchPage->layout());

    QLineEdit* searchBox = new QLineEdit();
    searchBox->setPlaceholderText("Search your music...");
    searchBox->setMinimumHeight(45);

    QLabel* searchInfo = new QLabel(
        "Your imported songs will appear in search results."
    );
    searchInfo->setObjectName("emptyMessage");

    searchLayout->insertWidget(2, searchBox);
    searchLayout->insertWidget(3, searchInfo);

    // LIBRARY PAGE
    QWidget* libraryPage = new QWidget();
    QVBoxLayout* libraryLayout = new QVBoxLayout(libraryPage);

    libraryLayout->setContentsMargins(35, 30, 35, 20);
    libraryLayout->setSpacing(15);

    QLabel* libraryHeading = new QLabel("Your Library");
    libraryHeading->setObjectName("heading");

    QLabel* librarySubtitle = new QLabel(
        "Your imported music will appear here."
    );
    librarySubtitle->setObjectName("subtitle");

    QPushButton* addMusicButton =
        new QPushButton("+   Add Music Folder");

    addMusicButton->setMinimumHeight(45);

    QListWidget* songList = new QListWidget();
    songList->setObjectName("songList");

    libraryLayout->addWidget(libraryHeading);
    libraryLayout->addWidget(librarySubtitle);
    libraryLayout->addWidget(addMusicButton);
    libraryLayout->addWidget(songList, 1);

    // CURRENT TRACK INFORMATION
    QFrame* trackInfo = new QFrame();

    QHBoxLayout* trackInfoLayout = new QHBoxLayout(trackInfo);
    trackInfoLayout->setContentsMargins(0, 0, 0, 0);
    trackInfoLayout->setSpacing(10);

    QLabel* albumArtwork = new QLabel("♫");
    albumArtwork->setFixedSize(48, 48);
    albumArtwork->setAlignment(Qt::AlignCenter);
    albumArtwork->setObjectName("albumArtwork");

    QVBoxLayout* trackTextLayout = new QVBoxLayout();
    trackTextLayout->setSpacing(3);

    QLabel* trackName = new QLabel("Nothing Playing");
    trackName->setObjectName("trackName");

    QLabel* artistName = new QLabel("Unknown Artist");
    artistName->setObjectName("artistName");

    QLabel* albumName = new QLabel("Unknown Album");
    albumName->setObjectName("artistName");

    trackTextLayout->addWidget(trackName);
    trackTextLayout->addWidget(artistName);
    trackTextLayout->addWidget(albumName);

    trackInfoLayout->addWidget(albumArtwork);
    trackInfoLayout->addLayout(trackTextLayout);

    // RESET TRACK INFORMATION
    auto resetTrackInfo = [
        trackName,
        artistName,
        albumName,
        albumArtwork
    ](QListWidgetItem* item) {

        if (!item) {
            return;
        }

        QString filePath = item->data(Qt::UserRole).toString();
        QString fallbackTitle = QFileInfo(filePath).completeBaseName();

        if (fallbackTitle.isEmpty()) {
            fallbackTitle = "Unknown Title";
        }

        trackName->setText(fallbackTitle);
        artistName->setText("Unknown Artist");
        albumName->setText("Unknown Album");

        albumArtwork->setPixmap(QPixmap());
        albumArtwork->setText("♫");
    };

    // BOTTOM PLAYER
    QFrame* player = new QFrame();
    player->setObjectName("player");
    player->setFixedHeight(115);

    QVBoxLayout* playerMainLayout = new QVBoxLayout(player);
    playerMainLayout->setContentsMargins(25, 8, 25, 8);
    playerMainLayout->setSpacing(2);

    QHBoxLayout* playerLayout = new QHBoxLayout();
    playerLayout->setSpacing(12);

    playerLayout->addWidget(trackInfo);
    playerLayout->addStretch();

    QPushButton* previousButton = new QPushButton("⏮");
    QPushButton* playButton = new QPushButton("▶");
    QPushButton* nextButton = new QPushButton("⏭");
    QPushButton* loopButton = new QPushButton("🔁");

    loopButton->setCheckable(true);
    loopButton->setToolTip("Repeat current song");

    playButton->setObjectName("playButton");

    playerLayout->addWidget(previousButton);
    playerLayout->addWidget(playButton);
    playerLayout->addWidget(nextButton);
    playerLayout->addWidget(loopButton);

    playerLayout->addStretch();

    QLabel* volumeLabel = new QLabel("Volume");

    QSlider* volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(70);
    volumeSlider->setFixedWidth(110);

    playerLayout->addWidget(volumeLabel);
    playerLayout->addWidget(volumeSlider);

    playerMainLayout->addLayout(playerLayout);

    // PROGRESS BAR
    QHBoxLayout* progressLayout = new QHBoxLayout();
    progressLayout->setSpacing(10);

    QLabel* currentTimeLabel = new QLabel("0:00");
    currentTimeLabel->setObjectName("timeLabel");

    QSlider* progressSlider = new QSlider(Qt::Horizontal);
    progressSlider->setObjectName("progressSlider");
    progressSlider->setRange(0, 0);
    progressSlider->setSingleStep(1000);
    progressSlider->setPageStep(5000);

    QLabel* durationLabel = new QLabel("0:00");
    durationLabel->setObjectName("timeLabel");

    progressLayout->addWidget(currentTimeLabel);
    progressLayout->addWidget(progressSlider, 1);
    progressLayout->addWidget(durationLabel);

    playerMainLayout->addLayout(progressLayout);

    // ADD PAGES
    QWidget* likedPage = createPage(
        "Liked Songs",
        "Songs you mark as favorites will appear here."
    );

    pages->addWidget(homePage);
    pages->addWidget(searchPage);
    pages->addWidget(libraryPage);
    pages->addWidget(likedPage);

    // NAVIGATION
    QObject::connect(homeButton, &QPushButton::clicked,
        [pages]() {
            pages->setCurrentIndex(0);
        });

    QObject::connect(searchButton, &QPushButton::clicked,
        [pages]() {
            pages->setCurrentIndex(1);
        });

    QObject::connect(libraryButton, &QPushButton::clicked,
        [pages]() {
            pages->setCurrentIndex(2);
        });

    QObject::connect(likedButton, &QPushButton::clicked,
        [pages]() {
            pages->setCurrentIndex(3);
        });

    // MUSIC LIBRARY SCANNING
    QSet<QString> importedSongPaths;

    auto scanMusicFolder = [
        songList,
        &importedSongPaths
    ](const QString& folder) {

        if (!QDir(folder).exists()) {
            return 0;
        }

        QStringList filters = {
            "*.mp3",
            "*.wav",
            "*.flac",
            "*.m4a",
            "*.aac",
            "*.ogg",
            "*.wma"
        };

        QDirIterator iterator(
            folder,
            filters,
            QDir::Files,
            QDirIterator::Subdirectories
        );

        int addedCount = 0;

        while (iterator.hasNext()) {
            QString filePath = iterator.next();

            if (importedSongPaths.contains(filePath)) {
                continue;
            }

            importedSongPaths.insert(filePath);

            QFileInfo fileInfo(filePath);

            QListWidgetItem* item =
                new QListWidgetItem(
                    "♫   " + fileInfo.completeBaseName()
                );

            item->setData(Qt::UserRole, filePath);

            songList->addItem(item);
            addedCount++;
        }

        return addedCount;
    };

    // RESTORE MUSIC FOLDERS FROM DATABASE
    QSqlQuery savedFolders("SELECT path FROM music_folders");

    while (savedFolders.next()) {
        scanMusicFolder(savedFolders.value(0).toString());
    }

    // ADD MUSIC FOLDER
    QObject::connect(addMusicButton, &QPushButton::clicked,
        [&window, songList, scanMusicFolder]() {

            QString folder = QFileDialog::getExistingDirectory(
                &window,
                "Select Your Music Folder"
            );

            if (folder.isEmpty()) {
                return;
            }

            QSqlQuery insertFolder;
            insertFolder.prepare(
                "INSERT OR IGNORE INTO music_folders (path) VALUES (?)"
            );
            insertFolder.addBindValue(folder);

            if (!insertFolder.exec()) {
                QMessageBox::warning(
                    &window,
                    "Database Error",
                    insertFolder.lastError().text()
                );
                return;
            }

            int songCount = scanMusicFolder(folder);

            if (songCount == 0) {
                QMessageBox::information(
                    &window,
                    "Music Import",
                    "No new supported music files were found in that folder."
                );
            }
            else {
                QMessageBox::information(
                    &window,
                    "Music Imported",
                    QString("Added %1 music files to your library.")
                        .arg(songCount)
                );
            }
        });

    // PLAY SELECTED SONG
    auto playSelectedSong = [
        mediaPlayer,
        songList,
        playButton,
        resetTrackInfo,
        progressSlider,
        currentTimeLabel,
        durationLabel
    ]() {

        QListWidgetItem* item = songList->currentItem();

        if (!item) {
            return;
        }

        QString filePath = item->data(Qt::UserRole).toString();

        resetTrackInfo(item);

        progressSlider->setValue(0);
        currentTimeLabel->setText("0:00");
        durationLabel->setText("0:00");

        mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        mediaPlayer->play();

        playButton->setText("Ⅱ");
    };

    // SINGLE CLICK SONG TO PLAY
    QObject::connect(
        songList,
        &QListWidget::itemClicked,
        [playSelectedSong](QListWidgetItem*) {
            playSelectedSong();
        });

    // PLAY / PAUSE
    QObject::connect(playButton, &QPushButton::clicked,
        [mediaPlayer, playSelectedSong, playButton]() {

            if (mediaPlayer->playbackState() ==
                QMediaPlayer::PlayingState) {

                mediaPlayer->pause();
                playButton->setText("▶");
            }
            else {
                if (mediaPlayer->source().isEmpty()) {
                    playSelectedSong();
                }
                else {
                    mediaPlayer->play();
                    playButton->setText("Ⅱ");
                }
            }
        });

    // NEXT SONG
    auto playNextSong = [
        mediaPlayer,
        songList,
        playButton,
        resetTrackInfo,
        progressSlider,
        currentTimeLabel,
        durationLabel
    ]() {

        int currentRow = songList->currentRow();

        if (currentRow + 1 >= songList->count()) {
            return;
        }

        songList->setCurrentRow(currentRow + 1);

        QListWidgetItem* item = songList->currentItem();

        if (!item) {
            return;
        }

        QString filePath = item->data(Qt::UserRole).toString();

        resetTrackInfo(item);

        progressSlider->setValue(0);
        currentTimeLabel->setText("0:00");
        durationLabel->setText("0:00");

        mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        mediaPlayer->play();

        playButton->setText("Ⅱ");
    };

    QObject::connect(nextButton, &QPushButton::clicked,
        playNextSong);

    // PREVIOUS SONG
    QObject::connect(previousButton, &QPushButton::clicked,
        [mediaPlayer, songList, playButton, resetTrackInfo,
         progressSlider, currentTimeLabel, durationLabel]() {

            int currentRow = songList->currentRow();

            if (currentRow <= 0) {
                return;
            }

            songList->setCurrentRow(currentRow - 1);

            QListWidgetItem* item = songList->currentItem();

            if (!item) {
                return;
            }

            QString filePath = item->data(Qt::UserRole).toString();

            resetTrackInfo(item);

            progressSlider->setValue(0);
            currentTimeLabel->setText("0:00");
            durationLabel->setText("0:00");

            mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
            mediaPlayer->play();

            playButton->setText("Ⅱ");
        });

    // AUTOMATICALLY PLAY NEXT SONG
    QObject::connect(
        mediaPlayer,
        &QMediaPlayer::mediaStatusChanged,
        [playNextSong](QMediaPlayer::MediaStatus status) {

            if (status == QMediaPlayer::EndOfMedia) {
                playNextSong();
            }
        });

    // LOOP CURRENT SONG
    QObject::connect(
        loopButton,
        &QPushButton::toggled,
        [mediaPlayer, loopButton](bool enabled) {

            if (enabled) {
                mediaPlayer->setLoops(QMediaPlayer::Infinite);

                loopButton->setStyleSheet(
                    "background-color: #444444;"
                    "color: #A0A0A0;"
                    "border-radius: 6px;"
                );
            }
            else {
                mediaPlayer->setLoops(1);
                loopButton->setStyleSheet("");
            }
        });

    // READ SONG METADATA
    QObject::connect(
        mediaPlayer,
        &QMediaPlayer::metaDataChanged,
        [mediaPlayer, songList, trackName, artistName,
         albumName, albumArtwork]() {

            QMediaMetaData metadata = mediaPlayer->metaData();

            QString title = metadata.stringValue(
                QMediaMetaData::Title
            );

            QString artist = metadata.value(
                QMediaMetaData::ContributingArtist
            ).toStringList().join(", ");

            if (artist.isEmpty()) {
                artist = metadata.stringValue(
                    QMediaMetaData::AlbumArtist
                );
            }

            QString album = metadata.stringValue(
                QMediaMetaData::AlbumTitle
            );

            if (title.isEmpty()) {
                QListWidgetItem* item = songList->currentItem();

                if (item) {
                    title = QFileInfo(
                        item->data(Qt::UserRole).toString()
                    ).completeBaseName();
                }

                if (title.isEmpty()) {
                    title = "Unknown Title";
                }
            }

            if (artist.isEmpty()) {
                artist = "Unknown Artist";
            }

            if (album.isEmpty()) {
                album = "Unknown Album";
            }

            trackName->setText(title);
            artistName->setText(artist);
            albumName->setText(album);

            QImage artwork = metadata.value(
                QMediaMetaData::CoverArtImage
            ).value<QImage>();

            if (!artwork.isNull()) {
                albumArtwork->setPixmap(
                    QPixmap::fromImage(artwork).scaled(
                        48,
                        48,
                        Qt::KeepAspectRatio,
                        Qt::SmoothTransformation
                    )
                );

                albumArtwork->setText("");
            }
            else {
                albumArtwork->setPixmap(QPixmap());
                albumArtwork->setText("♫");
            }
        });

    // PROGRESS BAR: UPDATE DURATION
    QObject::connect(
        mediaPlayer,
        &QMediaPlayer::durationChanged,
        [progressSlider, durationLabel](qint64 duration) {

            progressSlider->setRange(
                0,
                static_cast<int>(
                    qMin(duration, static_cast<qint64>(2147483647))
                )
            );

            durationLabel->setText(formatTime(duration));
        });

    // PROGRESS BAR: UPDATE POSITION
    QObject::connect(
        mediaPlayer,
        &QMediaPlayer::positionChanged,
        [progressSlider, currentTimeLabel](qint64 position) {

            if (!progressSlider->isSliderDown()) {
                progressSlider->setValue(
                    static_cast<int>(
                        qMin(position, static_cast<qint64>(2147483647))
                    )
                );
            }

            currentTimeLabel->setText(formatTime(position));
        });

    // PROGRESS BAR: SEEK
    QObject::connect(
        progressSlider,
        &QSlider::sliderMoved,
        [mediaPlayer](int position) {
            mediaPlayer->setPosition(position);
        });

    // VOLUME CONTROL
    QObject::connect(volumeSlider, &QSlider::valueChanged,
        [audioOutput](int value) {
            audioOutput->setVolume(value / 100.0);
        });

    // CREATE PLAYLIST UI
    auto createPlaylistUI = [
        &window,
        pages,
        playlistLayout,
        &database
    ](const QString& name, int playlistId) {

        QPushButton* playlistButton =
            new QPushButton("♫   " + name);

        playlistLayout->addWidget(playlistButton);

        QWidget* playlistPage = createPage(
            name,
            "Your playlist is ready for music."
        );

        pages->addWidget(playlistPage);

        QObject::connect(
            playlistButton,
            &QPushButton::clicked,
            [pages, playlistPage]() {
                pages->setCurrentWidget(playlistPage);
            });

        // RIGHT-CLICK TO DELETE PLAYLIST
        playlistButton->setContextMenuPolicy(Qt::CustomContextMenu);

        QObject::connect(
            playlistButton,
            &QPushButton::customContextMenuRequested,
            [&window, pages, playlistLayout, playlistButton,
             playlistPage, &database, name, playlistId](const QPoint&) {

                QMenu menu;

                QAction* deleteAction =
                    menu.addAction("Delete Playlist");

                QAction* selectedAction =
                    menu.exec(QCursor::pos());

                if (selectedAction != deleteAction) {
                    return;
                }

                QMessageBox::StandardButton answer =
                    QMessageBox::question(
                        &window,
                        "Delete Playlist",
                        "Are you sure you want to delete \"" +
                        name + "\"?",
                        QMessageBox::Yes | QMessageBox::No,
                        QMessageBox::No
                    );

                if (answer != QMessageBox::Yes) {
                    return;
                }

                QSqlQuery deleteQuery(database);

                deleteQuery.prepare(
                    "DELETE FROM playlists WHERE id = ?"
                );

                deleteQuery.addBindValue(playlistId);

                if (!deleteQuery.exec()) {
                    QMessageBox::warning(
                        &window,
                        "Database Error",
                        deleteQuery.lastError().text()
                    );
                    return;
                }

                if (pages->currentWidget() == playlistPage) {
                    pages->setCurrentIndex(0);
                }

                pages->removeWidget(playlistPage);
                playlistLayout->removeWidget(playlistButton);

                playlistPage->deleteLater();
                playlistButton->deleteLater();
            });
    };

    // RESTORE PLAYLISTS FROM DATABASE
    QSqlQuery savedPlaylists(
        "SELECT id, name FROM playlists ORDER BY id"
    );

    while (savedPlaylists.next()) {
        int id = savedPlaylists.value(0).toInt();
        QString name = savedPlaylists.value(1).toString();

        createPlaylistUI(name, id);
    }

    // CREATE PLAYLIST
    QObject::connect(createPlaylistButton, &QPushButton::clicked,
        [&window, &database, createPlaylistUI, pages]() {

            bool ok = false;

            QString name = QInputDialog::getText(
                &window,
                "Create Playlist",
                "Enter a playlist name:",
                QLineEdit::Normal,
                "",
                &ok
            );

            name = name.trimmed();

            if (!ok || name.isEmpty()) {
                return;
            }

            QSqlQuery checkQuery(database);

            checkQuery.prepare(
                "SELECT COUNT(*) FROM playlists WHERE name = ?"
            );

            checkQuery.addBindValue(name);

            if (!checkQuery.exec()) {
                QMessageBox::warning(
                    &window,
                    "Database Error",
                    checkQuery.lastError().text()
                );
                return;
            }

            checkQuery.next();

            if (checkQuery.value(0).toInt() > 0) {
                QMessageBox::information(
                    &window,
                    "Playlist Exists",
                    "A playlist with that name already exists."
                );
                return;
            }

            QSqlQuery insertQuery(database);

            insertQuery.prepare(
                "INSERT INTO playlists (name) VALUES (?)"
            );

            insertQuery.addBindValue(name);

            if (!insertQuery.exec()) {
                QMessageBox::warning(
                    &window,
                    "Database Error",
                    insertQuery.lastError().text()
                );
                return;
            }

            int newId = insertQuery.lastInsertId().toInt();

            createPlaylistUI(name, newId);

            pages->setCurrentIndex(pages->count() - 1);
        });

    // MAIN LAYOUT
    topLayout->addWidget(sidebar);
    topLayout->addWidget(pages, 1);

    mainLayout->addLayout(topLayout, 1);
    mainLayout->addWidget(player);

    // GRYONITE GRAY THEME
    window.setStyleSheet(R"(
        QMainWindow {
            background-color: #121212;
        }

        QWidget {
            color: #FFFFFF;
            font-family: "Segoe UI";
            font-size: 13px;
        }

        #sidebar {
            background-color: #181818;
        }

        #logo {
            color: #A0A0A0;
            font-size: 25px;
            font-weight: bold;
            letter-spacing: 3px;
        }

        #sectionLabel {
            color: #888888;
            font-size: 11px;
            font-weight: bold;
        }

        QPushButton {
            background-color: transparent;
            color: #B3B3B3;
            border: none;
            border-radius: 6px;
            padding: 11px;
            text-align: left;
        }

        QPushButton:hover {
            background-color: #292929;
            color: white;
        }

        #heading {
            font-size: 32px;
            font-weight: bold;
        }

        #subtitle {
            color: #999999;
            font-size: 14px;
        }

        #sectionHeading {
            font-size: 21px;
            font-weight: bold;
        }

        #albumCard {
            background-color: #202020;
            border-radius: 10px;
        }

        #albumCover {
            background-color: #303030;
            color: #A0A0A0;
            font-size: 42px;
            border-radius: 6px;
        }

        #albumArtwork {
            background-color: #303030;
            color: #A0A0A0;
            font-size: 25px;
            border-radius: 5px;
        }

        #albumName {
            font-weight: bold;
        }

        #artistName {
            color: #999999;
            font-size: 11px;
        }

        #emptyMessage {
            color: #888888;
            background-color: #1B1B1B;
            border-radius: 8px;
            padding: 20px;
        }

        #player {
            background-color: #181818;
            border-top: 1px solid #303030;
        }

        #trackName {
            font-weight: bold;
        }

        #playButton {
            background-color: #A0A0A0;
            color: #111111;
            border-radius: 20px;
            font-size: 17px;
            font-weight: bold;
            padding: 9px 13px;
        }

        #playButton:hover {
            background-color: #C0C0C0;
        }

        #timeLabel {
            color: #999999;
            font-size: 11px;
            min-width: 35px;
        }

        #progressSlider {
            background-color: transparent;
        }

        #progressSlider::groove:horizontal {
            background: #444444;
            height: 4px;
            border-radius: 2px;
        }

        #progressSlider::sub-page:horizontal {
            background: #A0A0A0;
            border-radius: 2px;
        }

        #progressSlider::handle:horizontal {
            background: #FFFFFF;
            width: 10px;
            height: 10px;
            margin: -3px 0;
            border-radius: 5px;
        }

        QLineEdit {
            background-color: #252525;
            color: white;
            border: 1px solid #444444;
            border-radius: 7px;
            padding: 12px;
        }

        QListWidget {
            background-color: #1B1B1B;
            border: none;
            border-radius: 8px;
            padding: 8px;
        }

        QListWidget::item {
            padding: 12px;
            border-radius: 5px;
        }

        QListWidget::item:hover {
            background-color: #303030;
        }

        QSlider::groove:horizontal {
            background: #444444;
            height: 4px;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            background: #A0A0A0;
            width: 12px;
            height: 12px;
            margin: -5px 0;
            border-radius: 6px;
        }

        QMenu {
            background-color: #252525;
            border: 1px solid #444444;
            padding: 5px;
        }

        QMenu::item {
            padding: 8px 25px;
        }

        QMenu::item:selected {
            background-color: #444444;
        }
    )");

    window.show();

    return app.exec();
}