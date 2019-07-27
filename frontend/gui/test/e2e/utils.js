import electron from 'electron'
import { Application } from 'spectron'

export default {
    afterEach() {
        this.timeout(10000)

        if (this.app && this.app.isRunning()) {
            return this.app.stop()
        }
    },

    beforeEach() {
        this.timeout(10000)
        this.app = new Application({
            path: electron,
            args: ['dist/electron/main.js'],
            startTimeout: 10000,
            waitTimeout: 10000
        })
        fakeDialog.apply(this.app)

        return this.app.start()
    },

    beforeEachAsm() {
        this.timeout(10000)
        this.app = new Application({
            path: electron,
            args: ['dist/electron/main.js'],
            startTimeout: 10000,
            waitTimeout: 10000
        })
        fakeDialog.apply(this.app)

        return this.app.start()
            .then(() => {
                fakeDialog.mock(
                    [ { method: 'showSaveDialog', value: generatedFilePath + asmFilename }
                    , { method: 'showOpenDialog', value: [generatedFilePath + 'hello_world.asm'] }
                    ]
                )
            })
    },

    beforeEachSim() {
        this.timeout(10000)
        this.app = new Application({
            path: electron,
            args: ['dist/electron/main.js'],
            startTimeout: 10000,
            waitTimeout: 10000
        })
        fakeDialog.apply(this.app)

        return this.app.start()
            .then(() => {
                fakeDialog.mock(
                    [ { method: 'showSaveDialog', value: generatedFilePath + asmFilename }
                    , { method: 'showOpenDialog', value: [generatedFilePath + objFilename] }
                    ]
                )
            })
    },

    saveAndAssemble(client, lines) {
        // Start testing command by clicking text editor
        var command = client.click('div.ace_content')

        // For each line in the 'lines' parameter,
        // add a command to type that line and to type 'Enter'
        const typeLineCmds = lines.map(line => (() => { return client.keys(line) }))
        for (var typeLineCmd of typeLineCmds) {
            command = command.then(typeLineCmd)
                .then(() => { return client.keys('Enter') })
        }

        // Add commands to save and build
        command = command.then(() => { return client.click('#save-button') })
            .then(() => { return client.click('#build-button') })

        // Return generated command
        return command
    }
}