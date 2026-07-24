void InputMethod::commitText(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    commitString(m_serial++, text);
}

static QByteArray keymapContentsForKeysym(xkb_keycode_t newKeycode, xkb_keysym_t customSym)
{
    char symName[64];
    if (xkb_keysym_get_name(customSym, symName, sizeof(symName)) <= 0) {
        qCWarning(KWIN_VIRTUALKEYBOARD) << "Could not find name for keysym" << customSym;
        return {};
    }
    const int keycode = newKeycode + 8;
    const QString keymap = QString::asprintf(
        "xkb_keymap {\n"
        "  xkb_keycodes \"custom\" {\n"
        "    <CSTM> = %d;\n"
        "  };\n"
        "  xkb_types \"(custom)\" { include \"complete\" };\n"
        "  xkb_compatibility \"custom\" { include \"complete\" };\n"
        "  xkb_symbols \"custom\" {\n"
        "    include \"pc+us\"\n"
        "    key <CSTM> { [ %s ] };\n"
        "  };\n"
        "};\n",
        keycode, symName);
    return keymap.toLatin1();
}

static void sendKeySymWithTemporaryKeymap(xkb_keysym_t keySym)
{
    static const uint syntheticKeyCode = 247;
    const QByteArray temporaryKeymap = keymapContentsForKeysym(syntheticKeyCode, keySym);
    if (temporaryKeymap.isEmpty()) {
        return;
    }
    auto *keyboard = waylandServer()->seat()->keyboard();
    if (!keyboard) {
        return;
    }
    keyboard->setKeymap(temporaryKeymap);
    const uint32_t serial = waylandServer()->seat()->nextSerial();
    waylandServer()->seat()->notifyKeyboardKey(syntheticKeyCode, KeyboardKeyState::Pressed, serial);
    waylandServer()->seat()->notifyKeyboardKey(syntheticKeyCode, KeyboardKeyState::Released, serial);
    keyboard->setKeymap(input()->keyboard()->xkb()->keymapContents());
}
