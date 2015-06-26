/*
  The MIT License (MIT)

  Copyright (c) 2015 Andrea Scarpino <me@andreascarpino.it>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.andreascarpino.sailhn 1.0

Page {
    property var kids

    readonly property int maxCommentsForPage: 30;
    property int showingCommentsCount: maxCommentsForPage;

    SilicaListView {
        id: comments
        anchors.fill: parent

        PullDownMenu {

            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    loadComments();
                    showingCommentsCount = maxCommentsForPage;
                }
            }
        }

        PushUpMenu {

            MenuItem {
                text: qsTr("Load more")
                onClicked: {
                    model.nextItems();
                    showingCommentsCount += maxCommentsForPage;
                }
            }
        }

        model: NewsModel {
            id: model

            onRowsInserted: {
                comments.pushUpMenu.visible = false;

                if (kids.length > showingCommentsCount) {
                    comments.pushUpMenu.visible = true;
                }
            }
        }

        header: PageHeader {
            title: qsTr("Comments")
        }

        delegate: CommentDelegate {}

        VerticalScrollDecorator {}
    }

    Component.onCompleted: loadComments()

    function loadComments() {
        model.loadComments(kids);
    }
}
