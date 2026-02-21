export async function runAot(code) {
    try {
        const response = await fetch('/api/run', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ code }),
        });

        const data = await response.json();

        if (!response.ok) {
            return {
                error: data.error || 'Server error',
                stdout: data.stdout || '',
                stderr: data.stderr || '',
                exitCode: data.exitCode || 1
            };
        }

        return {
            stdout: data.stdout || '',
            stderr: data.stderr || '',
            exitCode: data.exitCode || 0
        };
    } catch (err) {
        return {
            error: 'Network error or server unreachable',
            stdout: '',
            stderr: err.message,
            exitCode: -1
        };
    }
}
