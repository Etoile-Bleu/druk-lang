const express = require('express');
const cors = require('cors');
const fs = require('fs');
const path = require('path');
const { execFile } = require('child_process');
const crypto = require('crypto');

const app = express();
const port = 3000;

app.use(cors());
app.use(express.json());

const DRUK_COMPILER = '/app/compiler/build/druk';

app.post('/api/run', (req, res) => {
    const { code } = req.body;

    if (!code) {
        return res.status(400).json({ error: 'No code provided.' });
    }

    const sessionId = crypto.randomBytes(8).toString('hex');
    const sourceFilePath = path.join('/tmp', `script_${sessionId}.druk`);
    const exeFilePath = path.join('/tmp', `program_${sessionId}`);

    // 1. Write the code to a temporary file
    fs.writeFile(sourceFilePath, code, 'utf8', (err) => {
        if (err) {
            console.error(err);
            return res.status(500).json({ error: 'Failed to write source file.' });
        }

        // 2. Compile using Druk AOT
        execFile(DRUK_COMPILER, ['compile', sourceFilePath, '-o', exeFilePath], { timeout: 10000 }, (cmpErr, cmpStdout, cmpStderr) => {
            if (cmpErr) {
                // Compilation failed
                fs.unlink(sourceFilePath, () => { });
                return res.status(400).json({
                    error: 'Compilation Failed',
                    stdout: cmpStdout,
                    stderr: cmpStderr || cmpErr.message
                });
            }

            // 3. Execute the compiled binary
            execFile(exeFilePath, [], { timeout: 5000 }, (runErr, runStdout, runStderr) => {
                // Cleanup files
                fs.unlink(sourceFilePath, () => { });
                fs.unlink(exeFilePath, () => { });

                if (runErr && runErr.killed) {
                    return res.status(400).json({
                        error: 'Execution Timeout',
                        stdout: runStdout,
                        stderr: 'Execution timed out after 5 seconds.'
                    });
                }

                res.json({
                    stdout: runStdout,
                    stderr: runStderr,
                    exitCode: runErr ? runErr.code : 0
                });
            });
        });
    });
});

app.listen(port, () => {
    console.log(`Druk AOT Playground API listening on port ${port}`);
});
