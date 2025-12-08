import PySimpleGUI as sg


def main() -> None:
    layout = [
        [sg.Text("Ahoj z PySimpleGUI!")],
        [sg.Button("OK", key="-OK-"), sg.Button("Zrušit")]
    ]

    window = sg.Window("Ukázkové okno", layout)

    while True:
        event, values = window.read()
        if event in (sg.WINDOW_CLOSED, "-OK-", "Zrušit"):
            break

    window.close()


if __name__ == "__main__":
    main()

