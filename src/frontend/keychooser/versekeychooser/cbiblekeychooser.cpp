/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, https://bibletime.info/
*
* Copyright 1999-2021 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

#include "cbiblekeychooser.h"

#include <QDebug>
#include <QHBoxLayout>
#include "../../../backend/drivers/cswordbiblemoduleinfo.h"
#include "../../../backend/drivers/cswordmoduleinfo.h"
#include "../../../backend/keys/cswordversekey.h"
#include "../../../util/btassert.h"
#include "../../../util/btconnect.h"
#include "../bthistory.h"
#include "btbiblekeywidget.h"


CBibleKeyChooser::CBibleKeyChooser(const BtConstModuleList & modules,
                                   BTHistory * historyPtr,
                                   CSwordKey * key,
                                   QWidget * parent)
    : CKeyChooser(historyPtr, parent)
    , m_key(dynamic_cast<CSwordVerseKey *>(key))
{
    using CSBMI = CSwordBibleModuleInfo;

    w_ref = nullptr;
    setModules(modules, false);
    BT_ASSERT(!m_modules.empty());
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setDirection( QBoxLayout::LeftToRight );

    w_ref = new BtBibleKeyWidget(dynamic_cast<const CSBMI*>(m_modules.first()),
                                 m_key, this);
    setFocusProxy(w_ref);
    layout->addWidget(w_ref);

    BT_CONNECT(w_ref, &BtBibleKeyWidget::changed,
               [this](CSwordVerseKey * key) {
                   BT_ASSERT(m_key);
                   BT_ASSERT(key);

                   if (!updatesEnabled())
                       return;

                   setUpdatesEnabled(false);
                   m_key = key;
                   Q_EMIT keyChanged(m_key);

                   setUpdatesEnabled(true);
               });

    setKey(m_key); //set the key without changing it, setKey(key()) would change it

    BT_CONNECT(this, &CBibleKeyChooser::keyChanged, history(), &BTHistory::add);
}

CSwordKey* CBibleKeyChooser::key() {
    return m_key;
}

void CBibleKeyChooser::setKey(CSwordKey* key) {
    BT_ASSERT(dynamic_cast<CSwordVerseKey *>(key));
    if (dynamic_cast<CSwordVerseKey*>(key) == nullptr) return;

    m_key = dynamic_cast<CSwordVerseKey*>(key);
    w_ref->setKey(m_key);
    Q_EMIT keyChanged(m_key);
}

void CBibleKeyChooser::setModules(const BtConstModuleList &modules,
                                  bool refresh)
{
    using CSBMI = CSwordBibleModuleInfo;

    m_modules.clear();

    for (auto const * const mod : modules)
        if (mod->type() == CSwordModuleInfo::Bible
            || mod->type() == CSwordModuleInfo::Commentary)
            if (CSBMI const * const bible = dynamic_cast<CSBMI const *>(mod))
                m_modules.append(bible);

    // First time this is called we havnt set up w_ref.
    if (w_ref) w_ref->setModule(dynamic_cast<const CSwordBibleModuleInfo*>(m_modules.first()));
    if (refresh) refreshContent();
}

void CBibleKeyChooser::refreshContent() {
    setKey(m_key);
}

void CBibleKeyChooser::updateKey(CSwordKey* /*key*/) {
    w_ref->updateText();
}

void CBibleKeyChooser::handleHistoryMoved(QString const & newKey) {
    m_key->setKey(newKey);
    setKey(m_key);
}
