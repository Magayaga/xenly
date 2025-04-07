//
// XENLY - high-level and general-purpose programming language
// created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
//
// It is initially written in Zig programming language.
//
// It is available for Linux and Windows operating systems.
//
const std = @import("std");

pub const MAX_LINE_LENGTH = 1000;
const MAX_VARIABLES = 100;

const ValueType = enum {
    String,
    Integer,
    Float,
    Boolean,
};

const Value = union(ValueType) {
    String: []const u8,
    Integer: i64,
    Float: f64,
    Boolean: bool,
};

const spaceCharacters = [_]u8{' ', '\t', '\n', '\r'};

const Variable = struct {
    name: []const u8,
    value: Value,
};

var variables = std.ArrayList(Variable).init(std.heap.page_allocator);
var in_multi_line_comment = false;

pub fn processLine(line: []const u8) !void {
    const processed_line = try removeComments(line);
    if (processed_line.len > 0) {
        try interpretLine(processed_line);
    }
}

fn removeComments(line: []const u8) ![]const u8 {
    var result = std.ArrayList(u8).init(std.heap.page_allocator);
    defer result.deinit();

    var i: usize = 0;
    while (i < line.len) {
        if (in_multi_line_comment) {
            if (i + 1 < line.len and line[i] == '*' and line[i + 1] == '/') {
                in_multi_line_comment = false;
                i += 2;
            }
            
            else {
                i += 1;
            }
        }
        
        else if (i + 1 < line.len and line[i] == '/' and line[i + 1] == '/') {
            break;
        }
        
        else if (i + 1 < line.len and line[i] == '/' and line[i + 1] == '*') {
            in_multi_line_comment = true;
            i += 2;
        }
        
        else {
            try result.append(line[i]);
            i += 1;
        }
    }

    return result.toOwnedSlice();
}

fn interpretLine(line: []const u8) !void {
    const trimmed_line = std.mem.trim(u8, line, &spaceCharacters);

    if (std.mem.startsWith(u8, trimmed_line, "print(")) {
        const args = trimmed_line[5 .. trimmed_line.len - 1];
        try printFunction(args);
    }
    
    else if (std.mem.startsWith(u8, trimmed_line, "var ")) {
        try varDeclaration(trimmed_line[4..]);
    }
    
    else if (std.mem.startsWith(u8, trimmed_line, "bool ")) {
        try boolDeclaration(trimmed_line[5..]);
    }
}

fn printFunction(args: []const u8) !void {
    const trimmed_args = std.mem.trim(u8, args, &spaceCharacters);

    if (trimmed_args[0] == '"' and trimmed_args[trimmed_args.len - 1] == '"') {
        const str = trimmed_args[1 .. trimmed_args.len - 1];
        std.debug.print("{s}\n", .{str});
    }
    
    else {
        if (getVariableValue(trimmed_args)) |value| {
            switch (value) {
                .String => |s| std.debug.print("{s}\n", .{s}),
                .Integer => |i| std.debug.print("{d}\n", .{i}),
                .Float => |f| std.debug.print("{d}\n", .{f}),
                .Boolean => |b| std.debug.print("{}\n", .{b}),
            }
        }
        
        else {
            const result = try evaluateExpression(trimmed_args);
            std.debug.print("{d}\n", .{result});
        }
    }
}

fn varDeclaration(args: []const u8) !void {
    var iter = std.mem.splitAny(u8, args, "=");
    const name = std.mem.trim(u8, iter.next() orelse return error.InvalidDeclaration, &spaceCharacters);
    const value_str = std.mem.trim(u8, iter.next() orelse return error.InvalidDeclaration, &spaceCharacters);

    const value = if (value_str[0] == '"' and value_str[value_str.len - 1] == '"')
        Value{ .String = value_str[1 .. value_str.len - 1] }
    else if (std.mem.indexOfScalar(u8, value_str, '.')) |_|
        Value{ .Float = try std.fmt.parseFloat(f64, value_str) }
    else
        Value{ .Integer = try std.fmt.parseInt(i64, value_str, 10) };

    try variables.append(.{ .name = name, .value = value });
}

fn boolDeclaration(args: []const u8) !void {
    var iter = std.mem.splitAny(u8, args, "=");
    const name = std.mem.trim(u8, iter.next() orelse return error.InvalidDeclaration, &spaceCharacters);
    const value_str = std.mem.trim(u8, iter.next() orelse return error.InvalidDeclaration, &spaceCharacters);

    const value = Value{ .Boolean = try parseBool(value_str) };
    try variables.append(.{ .name = name, .value = value });
}

fn getVariableValue(name: []const u8) ?Value {
    for (variables.items) |var_| {
        if (std.mem.eql(u8, var_.name, name)) {
            return var_.value;
        }
    }
    return null;
}

fn evaluateExpression(expr: []const u8) !f64 {
    var iter = std.mem.tokenizeAny(u8, expr, " ");
    var result = try std.fmt.parseFloat(f64, iter.next() orelse return error.InvalidExpression);

    while (iter.next()) |op| {
        const num = try std.fmt.parseFloat(f64, iter.next() orelse return error.InvalidExpression);
        switch (op[0]) {
            '+' => result += num,
            '-' => result -= num,
            '*' => result *= num,
            '/' => result /= num,
            else => return error.InvalidOperator,
        }
    }

    return result;
}

fn parseBool(value: []const u8) !bool {
    if (std.mem.eql(u8, value, "true")) {
        return true;
    }
    
    else if (std.mem.eql(u8, value, "false")) {
        return false;
    }
    
    else {
        std.debug.print("Error: Invalid boolean value '{s}'. Defaulting to false.\n", .{value});
        return false;
    }
}