After relevant source changes, run `just fmt` (the canonical formatter) and then `just check` (formatting and clang-tidy audit). Fix every finding the check reports in code you touched before considering the change done.

Never commit or push unless explicitly requested.
